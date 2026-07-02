// tests/integration/test_cross_section_transit.cpp
//
// Integration test for a train crossing the Model-A cross-reference boundary
// between two loaded station scenarios (docs/plan_UID.md Etap 2C). Once both
// Gdynia Orlowo and Sopot are loaded via load_world(), the trunk-line section
// on each side directly references the other's UID as its neighbor — so a
// train travelling from Sopot toward GOr crosses via the ordinary
// section-to-section code path, with no boundary-node involved.

#include "engine/core/engine_state.hpp"
#include "engine/core/topology_loader.hpp"
#include "engine/core/train_fleet.hpp"
#include "engine/sim/train_sim.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <vector>

using namespace engine;
using namespace engine::core;
namespace fs = std::filesystem;

// Path to repository root — injected by CMake.
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

// L202 trunk-line sections either side of the Sopot/GOr Model-A cross-reference
// (see docs/plan_UID.md Etap 2C).
constexpr UID kSopotL202 = UID{2280627765267};      // szlak_sp_gor_l202_003 (Sopot side)
constexpr UID kSopotL202Prev = UID{2280627765266};  // its inward neighbor, i.e. side_a
constexpr UID kGorL202 = UID{2280627699729};        // szlak_sp_gor_l202_g01 (GOr side)

// Build a minimal TrainSimState positioned just before the far end of
// kSopotL202 (704 m long), heading toward GOr.
sim::TrainSimState make_fast_train_toward_gor()
{
    sim::TrainSimState st{};
    st.train_uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST, 0, 1);
    st.current_section_uid = kSopotL202;
    st.max_brake_kn = 50.0f;
    st.physics_params.total_mass_t = 100.0f;
    st.physics_params.max_traction_kn = 200.0f;
    st.physics_params.max_speed_ms = 33.3f;  // ~120 km/h
    st.physics_params.davis_A = 1.0f;
    st.physics_params.davis_B = 0.01f;
    st.physics_params.davis_C = 0.0001f;
    st.physics_state.position_m = 703.0f;  // 1 m before the section end
    st.physics_state.velocity_ms = 5.0f;   // 18 km/h — will cross within a few ticks
    st.driver_state = physics::DriverState::CRUISING;
    return st;
}

}  // namespace

TEST(CrossSectionTransit, TrainCrossesFromSopotIntoGdyniaOrlowo)
{
    EngineState state;
    load_world(state, gor_and_sopot_dirs());
    state.apply_track_section_occupancy(kSopotL202, TrackOccupancy::OCCUPIED, 4);

    TrainFleet fleet;
    fleet.add_train(make_fast_train_toward_gor(), kSopotL202Prev);

    for (int i = 0; i < 20; ++i)
    {
        fleet.tick_all(state, static_cast<uint64_t>(i), nullptr);
        if (state.find_track_section(kGorL202)->occupancy == TrackOccupancy::OCCUPIED)
            break;
    }

    const TrackSection* sopot_section = state.find_track_section(kSopotL202);
    const TrackSection* gor_section = state.find_track_section(kGorL202);
    ASSERT_NE(sopot_section, nullptr);
    ASSERT_NE(gor_section, nullptr);

    EXPECT_EQ(sopot_section->occupancy, TrackOccupancy::FREE);
    EXPECT_EQ(gor_section->occupancy, TrackOccupancy::OCCUPIED);
    EXPECT_EQ(fleet.size(), 1u);  // still an active train — no boundary node was involved
}
