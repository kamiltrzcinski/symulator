// tests/integration/test_spawn_despawn_transit.cpp
//
// End-to-end fleet-command flow over the real two-station world (GOr + Sopot):
//
//   1. resolve_spawn_at_boundary at GOr's southern L202 boundary node
//   2. TrainFleet::spawn — the adjacent trunk section becomes OCCUPIED and a
//      TrackSectionOccupancyChange (the DOMAIN_EVENT payload) is emitted
//   3. tick_all until the train crosses the Model-A cross-reference into Sopot,
//      capturing the occupancy-change sequence a client would receive
//   4. TrainFleet::despawn — the current section is freed
//
// Also covers loading of block_sections from the real scenario data.

#include "engine/core/engine_state.hpp"
#include "engine/core/spawn_resolver.hpp"
#include "engine/core/topology_loader.hpp"
#include "engine/core/train_fleet.hpp"
#include "engine/sim/train_sim.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <variant>
#include <vector>

using namespace engine;
using namespace engine::core;
namespace fs = std::filesystem;

#ifndef REPO_ROOT
#error "REPO_ROOT must be defined by CMake"
#endif

namespace
{

std::vector<fs::path> gor_and_sopot_dirs()
{
    return {
        fs::path(REPO_ROOT) / "scenarios" / "gdynia_orlowo",
        fs::path(REPO_ROOT) / "scenarios" / "sopot",
    };
}

// GOr southern L202 boundary node and the trunk sections either side of the
// GOr↔Sopot cross-reference (see docs/plan_UID.md Etap 2C).
constexpr UID kGorL202SouthBnd = UID{2302102536193};  // l202_granica_poludniowa
constexpr UID kGorL202G03 = UID{2280627699731};       // szlak_sp_gor_l202_g03 (adjacent)
constexpr UID kGorL202G02 = UID{2280627699730};
constexpr UID kGorL202G01 = UID{2280627699729};
constexpr UID kSopotL202 = UID{2280627765267};  // szlak_sp_gor_l202_003 (Sopot side)

constexpr UID kTrainUid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST, 0, 1);
constexpr int kAxles = 16;

sim::TrainSimState make_train(UID section_uid, float position_m)
{
    sim::TrainSimState st{};
    st.train_uid = kTrainUid;
    st.current_section_uid = section_uid;
    st.max_brake_kn = 50.0f;
    st.total_axles = kAxles;
    st.physics_params.total_mass_t = 100.0f;
    st.physics_params.max_traction_kn = 200.0f;
    st.physics_params.max_speed_ms = 33.3f;
    st.physics_params.davis_A = 1.0f;
    st.physics_params.davis_B = 0.01f;
    st.physics_params.davis_C = 0.0001f;
    st.physics_state.position_m = position_m;
    st.physics_state.velocity_ms = 5.0f;
    st.driver_state = physics::DriverState::CRUISING;
    return st;
}

std::vector<TrackSectionOccupancyChange> occupancy_changes(
    const std::vector<DeviceStateChange>& changes)
{
    std::vector<TrackSectionOccupancyChange> out;
    for (const auto& c : changes)
        if (const auto* occ = std::get_if<TrackSectionOccupancyChange>(&c))
            out.push_back(*occ);
    return out;
}

}  // namespace

TEST(SpawnDespawnTransit, FullFlowAcrossTheRealWorld)
{
    EngineState state;
    load_world(state, gor_and_sopot_dirs());

    // ── 1. Resolve the spawn point at GOr's southern boundary ────────────────
    const auto resolved = resolve_spawn_at_boundary(state, kGorL202SouthBnd);
    const auto* point = std::get_if<SpawnPoint>(&resolved);
    ASSERT_NE(point, nullptr) << "expected a unique free section behind the boundary";
    EXPECT_EQ(point->section_uid, kGorL202G03);
    EXPECT_EQ(point->from_uid, kGorL202SouthBnd);

    // ── 2. Spawn near the far end of the section, heading toward Sopot ───────
    TrainFleet fleet;
    std::vector<DeviceStateChange> changes;
    std::vector<PipEvent> pip_events;
    const float section_length = state.find_track_section(point->section_uid)->length_m;
    const auto spawn_error =
        fleet.spawn(state, make_train(point->section_uid, section_length - 1.0f), point->from_uid,
                    changes, pip_events);
    ASSERT_FALSE(spawn_error.has_value()) << *spawn_error;

    auto occ = occupancy_changes(changes);
    ASSERT_EQ(occ.size(), 1u);
    EXPECT_EQ(occ[0].uid, kGorL202G03);
    EXPECT_EQ(occ[0].occupancy, TrackOccupancy::OCCUPIED);
    EXPECT_EQ(occ[0].axle_count, kAxles);
    EXPECT_EQ(occ[0].train_uid, kTrainUid);
    EXPECT_EQ(state.find_track_section(kGorL202G03)->axle_count, kAxles);

    // ── 3. Tick until the train reaches the Sopot side of the cross-reference ─
    std::vector<TrackSectionOccupancyChange> transit_changes;
    for (int i = 0; i < 2000 && fleet.size() == 1; ++i)
    {
        auto tick_changes =
            occupancy_changes(fleet.tick_all(state, static_cast<uint64_t>(i), nullptr));
        transit_changes.insert(transit_changes.end(), tick_changes.begin(), tick_changes.end());
        if (state.find_track_section(kSopotL202)->occupancy == TrackOccupancy::OCCUPIED)
            break;
    }

    EXPECT_EQ(state.find_track_section(kSopotL202)->occupancy, TrackOccupancy::OCCUPIED);
    EXPECT_EQ(state.find_track_section(kSopotL202)->axle_count, kAxles);
    EXPECT_EQ(fleet.size(), 1u);

    // The client-visible sequence: G03 freed, G02/G01 occupied then freed,
    // Sopot-side section occupied last.
    ASSERT_GE(transit_changes.size(), 2u);
    EXPECT_EQ(transit_changes.front().uid, kGorL202G03);
    EXPECT_EQ(transit_changes.front().occupancy, TrackOccupancy::FREE);
    EXPECT_EQ(transit_changes.back().uid, kSopotL202);
    EXPECT_EQ(transit_changes.back().occupancy, TrackOccupancy::OCCUPIED);

    const bool visited_g02 =
        std::any_of(transit_changes.begin(), transit_changes.end(), [](const auto& c)
                    { return c.uid == kGorL202G02 && c.occupancy == TrackOccupancy::OCCUPIED; });
    const bool visited_g01 =
        std::any_of(transit_changes.begin(), transit_changes.end(), [](const auto& c)
                    { return c.uid == kGorL202G01 && c.occupancy == TrackOccupancy::OCCUPIED; });
    EXPECT_TRUE(visited_g02);
    EXPECT_TRUE(visited_g01);

    // ── 4. Despawn — the train's current section must be freed ───────────────
    changes.clear();
    pip_events.clear();
    const UID current_section = kSopotL202;
    const auto despawn_error = fleet.despawn(state, kTrainUid, changes, pip_events);
    ASSERT_FALSE(despawn_error.has_value()) << *despawn_error;
    EXPECT_TRUE(fleet.empty());
    EXPECT_EQ(state.find_track_section(current_section)->occupancy, TrackOccupancy::FREE);
    EXPECT_EQ(state.find_track_section(current_section)->axle_count, 0);

    occ = occupancy_changes(changes);
    ASSERT_EQ(occ.size(), 1u);
    EXPECT_EQ(occ[0].uid, current_section);
    EXPECT_EQ(occ[0].occupancy, TrackOccupancy::FREE);
}

TEST(SpawnDespawnTransit, BlockSectionsLoadedFromRealScenarios)
{
    EngineState state;
    load_world(state, gor_and_sopot_dirs());

    // GOr: blk_l202_gor_sp / blk_l250_gor_sp;  Sopot: blk_l202_sp_gor / blk_l250_sp_gor.
    const auto* gor_l202 = state.find_block_section(UID{2297807568897});
    ASSERT_NE(gor_l202, nullptr);
    EXPECT_EQ(gor_l202->pid, "blk_l202_gor_sp");
    EXPECT_EQ(gor_l202->type_id, "SHL-12");
    EXPECT_EQ(gor_l202->line_number, 202);
    EXPECT_EQ(gor_l202->state, BlockSectionState::CLOSED);
    EXPECT_EQ(gor_l202->direction, BlockDirectionState::NEUTRAL);
    EXPECT_EQ(gor_l202->szlak_section_uids.size(), 3u);

    const auto* sp_l202 = state.find_block_section(UID{2297807634433});
    ASSERT_NE(sp_l202, nullptr);
    EXPECT_EQ(sp_l202->pid, "blk_l202_sp_gor");

    // The two sides of the same physical line reference each other's station.
    EXPECT_EQ(gor_l202->neighbor_station_uid, sp_l202->station_uid);
    EXPECT_EQ(sp_l202->neighbor_station_uid, gor_l202->station_uid);

    ASSERT_NE(state.find_block_section(UID{2297807568898}), nullptr);  // GOr L250
    ASSERT_NE(state.find_block_section(UID{2297807634434}), nullptr);  // Sp L250
}
