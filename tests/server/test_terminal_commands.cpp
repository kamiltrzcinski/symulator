// tests/server/test_terminal_commands.cpp
//
// Tests for SpawnCommand / DespawnCommand / TrainsCommand: argument parsing
// (UID vs pID), synchronous validation against the snapshot, and the enqueued
// FleetCommand payloads.  Uses a fake enqueue function — no EngineLoop.

#include "server/terminal/despawn_command.hpp"
#include "server/terminal/lookup.hpp"
#include "server/terminal/spawn_command.hpp"
#include "server/terminal/trains_command.hpp"

#include "engine/core/engine_snapshot.hpp"
#include "engine/core/fleet_registry.hpp"

#include <gtest/gtest.h>

#include <tests/common/file_test_helpers.hpp>

#include <filesystem>
#include <memory>
#include <variant>
#include <vector>

namespace
{

using namespace engine::core;
using namespace server::terminal;

constexpr UID kBnd = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 1);
constexpr UID kSection = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
constexpr UID kSta = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);

// Minimal fleet data on disk: one vehicle type, one vehicle, one consist.
const FleetRegistry& test_fleet()
{
    static FleetRegistry fleet = []
    {
        static tests::common::TemporaryDirectory tmp{"symulator_term_cmd_"};
        const auto root = tmp.path();

        tests::common::write_text(root / "vehicle-types" / "emu_unit" / "motor" / "en57.json",
                                  R"json({
  "uid": 1103806595073,
  "typeName": "EN57",
  "vehicleType": "EMU_UNIT",
  "vehicleSubtype": "MOTOR",
  "lengthM": 65.0,
  "axleCount": 16,
  "massEmptyT": 126.5,
  "maxSpeedKmh": 110,
  "brakingLambdaPct": 100,
  "powerKW": 1000.0,
  "tractionForceKN": 120.0
})json");
        tests::common::write_text(root / "vehicles" / "emu_unit" / "en57-001" / "vehicle.json",
                                  R"json({
  "uid": 1108101562369,
  "pID": "EN57-001",
  "type_uid": 1103806595073,
  "displayName": "EN57-001",
  "tractionStatus": "OPERATIONAL"
})json");
        tests::common::write_text(root / "trains" / "passenger" / "consist.json", R"json({
  "uid": 1112396529665,
  "pID": "TEST-CONSIST",
  "displayName": "Test consist",
  "trainCategory": "PASSENGER",
  "vehicle_uids": [1108101562369]
})json");

        FleetRegistry f;
        f.load(root);
        return f;
    }();
    return fleet;
}

constexpr UID kConsist{1112396529665};

// Snapshot with one free section behind a boundary node.
std::shared_ptr<EngineSnapshot> make_world_snapshot()
{
    auto snap = std::make_shared<EngineSnapshot>();
    snap->session = "TERM_TEST";
    snap->boundary_nodes[kBnd] = BoundaryNode{kBnd, "bnd-south", kSta, ""};

    TrackSection ts{};
    ts.uid = kSection;
    ts.pid = "tor1";
    ts.station_uid = kSta;
    ts.length_m = 500.0f;
    ts.side_a.neighbor_uid = kBnd;
    snap->track_sections[kSection] = ts;
    return snap;
}

struct CommandFixture
{
    AtomicSnapshot snapshot;
    std::vector<FleetCommand> enqueued;

    SpawnCommand::EnqueueFn enqueue_fn()
    {
        return [this](FleetCommand cmd) { enqueued.push_back(std::move(cmd)); };
    }
};

// ── SpawnCommand ──────────────────────────────────────────────────────────────

TEST(SpawnCommand, UsageOnWrongArgCount)
{
    CommandFixture f;
    SpawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());
    EXPECT_NE(cmd.execute({}).find("usage:"), std::string::npos);
    EXPECT_TRUE(f.enqueued.empty());
}

TEST(SpawnCommand, FailsWithoutSnapshot)
{
    CommandFixture f;
    SpawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());
    EXPECT_NE(cmd.execute({"TEST-CONSIST", "bnd-south"}).find("snapshot"), std::string::npos);
}

TEST(SpawnCommand, SpawnsByPidAndEnqueuesRequest)
{
    CommandFixture f;
    f.snapshot.publish(make_world_snapshot());
    SpawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());

    const auto out = cmd.execute({"TEST-CONSIST", "bnd-south"});
    EXPECT_NE(out.find("spawn queued"), std::string::npos) << out;

    ASSERT_EQ(f.enqueued.size(), 1u);
    const auto* req = std::get_if<SpawnRequest>(&f.enqueued[0]);
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->initial.train_uid, kConsist);
    EXPECT_EQ(req->initial.current_section_uid, kSection);
    EXPECT_EQ(req->initial.total_axles, 16);
    EXPECT_EQ(req->from_uid, kBnd);
}

TEST(SpawnCommand, SpawnsByDecimalUid)
{
    CommandFixture f;
    f.snapshot.publish(make_world_snapshot());
    SpawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());

    const auto out = cmd.execute({std::to_string(kConsist.value), std::to_string(kBnd.value)});
    EXPECT_NE(out.find("spawn queued"), std::string::npos) << out;
    EXPECT_EQ(f.enqueued.size(), 1u);
}

TEST(SpawnCommand, UnknownConsistReported)
{
    CommandFixture f;
    f.snapshot.publish(make_world_snapshot());
    SpawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());

    EXPECT_NE(cmd.execute({"NOPE", "bnd-south"}).find("unknown consist"), std::string::npos);
    EXPECT_TRUE(f.enqueued.empty());
}

TEST(SpawnCommand, UnknownBoundaryReported)
{
    CommandFixture f;
    f.snapshot.publish(make_world_snapshot());
    SpawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());

    EXPECT_NE(cmd.execute({"TEST-CONSIST", "nowhere"}).find("unknown boundary"), std::string::npos);
    EXPECT_TRUE(f.enqueued.empty());
}

TEST(SpawnCommand, OccupiedSectionRejectedSynchronously)
{
    CommandFixture f;
    auto snap = make_world_snapshot();
    snap->track_sections[kSection].occupancy = TrackOccupancy::OCCUPIED;
    f.snapshot.publish(std::move(snap));
    SpawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());

    EXPECT_NE(cmd.execute({"TEST-CONSIST", "bnd-south"}).find("spawn rejected"), std::string::npos);
    EXPECT_TRUE(f.enqueued.empty());
}

TEST(SpawnCommand, AlreadyActiveTrainRejected)
{
    CommandFixture f;
    auto snap = make_world_snapshot();
    snap->trains.push_back(
        TrainSnapshot{.uid = kConsist, .section_uid = kSection, .vehicle_uids = {}});
    f.snapshot.publish(std::move(snap));
    SpawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());

    EXPECT_NE(cmd.execute({"TEST-CONSIST", "bnd-south"}).find("already active"), std::string::npos);
    EXPECT_TRUE(f.enqueued.empty());
}

// ── DespawnCommand ────────────────────────────────────────────────────────────

TEST(DespawnCommand, DespawnsActiveTrainByPid)
{
    CommandFixture f;
    auto snap = make_world_snapshot();
    snap->trains.push_back(
        TrainSnapshot{.uid = kConsist, .section_uid = kSection, .vehicle_uids = {}});
    f.snapshot.publish(std::move(snap));
    DespawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());

    const auto out = cmd.execute({"TEST-CONSIST"});
    EXPECT_NE(out.find("despawn queued"), std::string::npos) << out;

    ASSERT_EQ(f.enqueued.size(), 1u);
    const auto* req = std::get_if<DespawnRequest>(&f.enqueued[0]);
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->train_uid, kConsist);
}

TEST(DespawnCommand, InactiveTrainReported)
{
    CommandFixture f;
    f.snapshot.publish(make_world_snapshot());  // no trains
    DespawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());

    EXPECT_NE(cmd.execute({"TEST-CONSIST"}).find("no active train"), std::string::npos);
    EXPECT_TRUE(f.enqueued.empty());
}

TEST(DespawnCommand, UsageOnWrongArgCount)
{
    CommandFixture f;
    DespawnCommand cmd(test_fleet(), f.snapshot, f.enqueue_fn());
    EXPECT_NE(cmd.execute({}).find("usage:"), std::string::npos);
}

// ── TrainsCommand ─────────────────────────────────────────────────────────────

TEST(TrainsCommand, ListsActiveTrains)
{
    CommandFixture f;
    auto snap = make_world_snapshot();
    snap->trains.push_back(TrainSnapshot{.uid = kConsist,
                                         .section_uid = kSection,
                                         .speed_kmh = 55.0f,
                                         .total_axles = 16,
                                         .vehicle_uids = {}});
    f.snapshot.publish(std::move(snap));
    TrainsCommand cmd(f.snapshot);

    const auto out = cmd.execute({});
    EXPECT_NE(out.find("active trains (1)"), std::string::npos) << out;
    EXPECT_NE(out.find(std::to_string(kConsist.value)), std::string::npos);
}

TEST(TrainsCommand, NoTrains)
{
    CommandFixture f;
    f.snapshot.publish(make_world_snapshot());
    TrainsCommand cmd(f.snapshot);
    EXPECT_EQ(cmd.execute({}), "no active trains");
}

// ── lookup helpers ────────────────────────────────────────────────────────────

TEST(TerminalLookup, ParseUintRejectsMixedInput)
{
    EXPECT_EQ(parse_uint("123").value_or(0), 123u);
    EXPECT_FALSE(parse_uint("12a").has_value());
    EXPECT_FALSE(parse_uint("EN57-001").has_value());
    EXPECT_FALSE(parse_uint("").has_value());
}

}  // namespace
