#include <gtest/gtest.h>

#include <engine/core/command.hpp>
#include <engine/core/control_system.hpp>
#include <engine/core/engine_loop.hpp>
#include <engine/core/engine_snapshot.hpp>
#include <engine/core/engine_state.hpp>
#include <engine/core/priority_command_queue.hpp>
#include <engine/core/track_model.hpp>
#include <engine/core/types.hpp>

#include <engine/sim/train_sim.hpp>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <variant>
#include <vector>

namespace
{

using namespace engine::core;

// ── Stub IControlSystem ───────────────────────────────────────────────────────
// Accepts all commands and produces no state changes.

class StubControlSystem final : public IControlSystem
{
public:
    std::string system_id() const override { return "stub"; }

    std::optional<InterlockingViolation> check_command(const IStateView&,
                                                       const Command&) const override
    {
        return std::nullopt;
    }

    std::vector<DeviceStateChange> execute_command(const IStateView&, const Command&) override
    {
        ++execute_count;
        return {};
    }

    std::vector<DeviceStateChange> on_tick(const IStateView&, uint64_t) override
    {
        ++tick_count;
        return {};
    }

    std::vector<std::string> supported_command_types() const override { return {}; }

    std::atomic<int> execute_count{0};
    std::atomic<int> tick_count{0};
};

// Reject all commands unconditionally.
class RejectAllControlSystem final : public IControlSystem
{
public:
    std::string system_id() const override { return "reject-all"; }

    std::optional<InterlockingViolation> check_command(const IStateView&,
                                                       const Command&) const override
    {
        InterlockingViolation v;
        v.reason_code = 2;  // SAFETY_BLOCK
        v.reason_text = "Test rejection";
        return v;
    }

    std::vector<DeviceStateChange> execute_command(const IStateView&, const Command&) override
    {
        return {};
    }
    std::vector<DeviceStateChange> on_tick(const IStateView&, uint64_t) override { return {}; }
    std::vector<std::string> supported_command_types() const override { return {}; }
};

// ── Helpers ───────────────────────────────────────────────────────────────────

// Wait until the predicate returns true or timeout elapses.
template<typename Pred>
bool wait_for(Pred pred, std::chrono::milliseconds timeout = std::chrono::milliseconds{500})
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (pred())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds{5});
    }
    return false;
}

EngineState make_state()
{
    EngineState st;
    st.set_session_id("LOOP_TEST");
    st.set_current_tick(0);
    return st;
}

// ── Fleet-command helpers ─────────────────────────────────────────────────────

constexpr UID kBnd = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 1);
constexpr UID kSection = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
constexpr UID kSta = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
constexpr UID kTrain = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST, 0, 7);

// One very long section behind a boundary node, so a spawned train stays on it
// for the whole test duration.
EngineState make_state_with_section()
{
    EngineState st = make_state();
    st.insert_boundary_node(BoundaryNode{kBnd, "bnd", kSta, ""});

    TrackSection ts{};
    ts.uid = kSection;
    ts.pid = "long-section";
    ts.station_uid = kSta;
    ts.length_m = 10000.0f;
    ts.max_speed_kmh = 40;
    ts.side_a.neighbor_uid = kBnd;
    st.insert_track_section(ts);
    return st;
}

engine::sim::TrainSimState make_stationary_train()
{
    engine::sim::TrainSimState train{};
    train.train_uid = kTrain;
    train.current_section_uid = kSection;
    train.total_axles = 8;
    train.physics_params.total_mass_t = 100.0f;
    train.physics_params.max_speed_ms = 20.0f;
    return train;
}

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST(EngineLoop, StartAndStopClean)
{
    auto st = make_state();
    StubControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    EngineLoop loop(st, ctrl, q, snap);
    EXPECT_FALSE(loop.is_running());
    loop.start();
    EXPECT_TRUE(loop.is_running());
    loop.stop();
    EXPECT_FALSE(loop.is_running());
}

TEST(EngineLoop, TickCounterAdvances)
{
    auto st = make_state();
    StubControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    EngineLoop loop(st, ctrl, q, snap);
    loop.start();

    // Wait for at least 3 ticks (each 50 ms → ~150 ms + tolerance)
    EXPECT_TRUE(wait_for([&] { return st.current_tick() >= 3; }, std::chrono::milliseconds{600}));
    loop.stop();
    EXPECT_GE(st.current_tick(), 3u);
}

TEST(EngineLoop, SnapshotPublishedAfterTick)
{
    auto st = make_state();
    StubControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    EngineLoop loop(st, ctrl, q, snap);
    loop.start();

    EXPECT_TRUE(wait_for([&] { return snap.load() != nullptr; }, std::chrono::milliseconds{300}));
    loop.stop();

    const auto s = snap.load();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->session, "LOOP_TEST");
    EXPECT_GE(s->tick, 1u);
}

TEST(EngineLoop, OnTickCalledEachTick)
{
    auto st = make_state();
    StubControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    EngineLoop loop(st, ctrl, q, snap);
    loop.start();

    EXPECT_TRUE(
        wait_for([&] { return ctrl.tick_count.load() >= 2; }, std::chrono::milliseconds{500}));
    loop.stop();
    EXPECT_GE(ctrl.tick_count.load(), 2);
}

TEST(EngineLoop, CommandIsExecuted)
{
    auto st = make_state();
    // Insert a switch so the command can be valid
    Switch sw;
    sw.uid = UID{0x020400000001ULL};
    sw.pid = "zwrX";
    st.insert_switch(sw);

    StubControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    EngineLoop loop(st, ctrl, q, snap);
    loop.start();

    // Push one command
    EnvelopedCommand cmd;
    cmd.meta.seq_id = 1;
    cmd.meta.priority = CommandPriority::NORMAL;
    cmd.payload = SetSwitchPositionCmd{UID{0x020400000001ULL}, SwitchPosition::DIVERGENT};
    q.push(std::move(cmd), CommandPriority::NORMAL);

    EXPECT_TRUE(
        wait_for([&] { return ctrl.execute_count.load() >= 1; }, std::chrono::milliseconds{400}));
    loop.stop();
    EXPECT_EQ(ctrl.execute_count.load(), 1);
}

TEST(EngineLoop, RejectedCommandInvokesNakCallback)
{
    auto st = make_state();
    RejectAllControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    std::atomic<int> nak_count{0};
    EngineLoop::NakCallback nak_cb = [&](const EnvelopedCommand&, const InterlockingViolation&)
    { ++nak_count; };

    EngineLoop loop(st, ctrl, q, snap, std::move(nak_cb));
    loop.start();

    EnvelopedCommand cmd;
    cmd.meta.seq_id = 42;
    cmd.meta.priority = CommandPriority::NORMAL;
    cmd.payload = SetSignalAspectCmd{UID{0x020500000001ULL}, SignalAspect::S2_PROCEED};
    q.push(std::move(cmd), CommandPriority::NORMAL);

    EXPECT_TRUE(wait_for([&] { return nak_count.load() >= 1; }, std::chrono::milliseconds{400}));
    loop.stop();
    EXPECT_GE(nak_count.load(), 1);
}

TEST(EngineLoop, FleetCommand_SpawnAppearsInNextTicks)
{
    auto st = make_state_with_section();
    StubControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    EngineLoop loop(st, ctrl, q, snap);
    loop.start();

    loop.enqueue_fleet_command(SpawnRequest{make_stationary_train(), kBnd});

    EXPECT_TRUE(wait_for(
        [&]
        {
            const auto s = snap.load();
            return s && s->trains.size() == 1;
        },
        std::chrono::milliseconds{600}));
    loop.stop();

    const auto s = snap.load();
    ASSERT_NE(s, nullptr);
    ASSERT_EQ(s->trains.size(), 1u);
    EXPECT_EQ(s->trains[0].uid, kTrain);
    EXPECT_EQ(s->trains[0].total_axles, 8);
    EXPECT_EQ(s->track_sections.at(kSection).occupancy, TrackOccupancy::OCCUPIED);
    EXPECT_EQ(s->track_sections.at(kSection).axle_count, 8);
}

TEST(EngineLoop, FleetCommand_SpawnOntoOccupiedSectionRejected)
{
    auto st = make_state_with_section();
    st.apply_track_section_occupancy(kSection, TrackOccupancy::OCCUPIED, 4);

    StubControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    EngineLoop loop(st, ctrl, q, snap);
    loop.start();

    loop.enqueue_fleet_command(SpawnRequest{make_stationary_train(), kBnd});

    // Give the loop a few ticks to drain the request, then confirm no train.
    EXPECT_TRUE(wait_for([&] { return st.current_tick() >= 3; }, std::chrono::milliseconds{600}));
    loop.stop();

    const auto s = snap.load();
    ASSERT_NE(s, nullptr);
    EXPECT_TRUE(s->trains.empty());
}

TEST(EngineLoop, FleetCommand_DespawnFreesSection)
{
    auto st = make_state_with_section();
    StubControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    std::vector<DeviceStateChange> captured;
    std::mutex captured_mutex;
    EngineLoop::StateChangesCallback changes_cb = [&](const std::vector<DeviceStateChange>& changes)
    {
        std::scoped_lock lock{captured_mutex};
        captured.insert(captured.end(), changes.begin(), changes.end());
    };

    EngineLoop loop(st, ctrl, q, snap, nullptr, std::move(changes_cb));
    loop.start();

    loop.enqueue_fleet_command(SpawnRequest{make_stationary_train(), kBnd});
    EXPECT_TRUE(wait_for(
        [&]
        {
            const auto s = snap.load();
            return s && s->trains.size() == 1;
        },
        std::chrono::milliseconds{600}));

    loop.enqueue_fleet_command(DespawnRequest{kTrain});
    EXPECT_TRUE(wait_for(
        [&]
        {
            const auto s = snap.load();
            return s && s->trains.empty();
        },
        std::chrono::milliseconds{600}));
    loop.stop();

    const auto s = snap.load();
    ASSERT_NE(s, nullptr);
    EXPECT_EQ(s->track_sections.at(kSection).occupancy, TrackOccupancy::FREE);
    EXPECT_EQ(s->track_sections.at(kSection).axle_count, 0);

    // The occupancy changes must have reached the changes callback (the
    // DOMAIN_EVENT broadcast path): OCCUPIED on spawn, FREE on despawn.
    std::scoped_lock lock{captured_mutex};
    int occupied = 0;
    int freed = 0;
    for (const auto& change : captured)
    {
        if (const auto* occ = std::get_if<TrackSectionOccupancyChange>(&change))
        {
            if (occ->uid == kSection && occ->occupancy == TrackOccupancy::OCCUPIED)
                ++occupied;
            if (occ->uid == kSection && occ->occupancy == TrackOccupancy::FREE)
                ++freed;
        }
    }
    EXPECT_EQ(occupied, 1);
    EXPECT_EQ(freed, 1);
}

TEST(EngineLoop, MultipleStartCallsAreIdempotent)
{
    auto st = make_state();
    StubControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    EngineLoop loop(st, ctrl, q, snap);
    loop.start();
    loop.start();  // second call should be a no-op
    EXPECT_TRUE(loop.is_running());
    loop.stop();
}

TEST(EngineLoop, StopWithoutStartIsNoOp)
{
    auto st = make_state();
    StubControlSystem ctrl;
    PriorityCommandQueue<EnvelopedCommand> q;
    AtomicSnapshot snap;

    EngineLoop loop(st, ctrl, q, snap);
    EXPECT_NO_THROW(loop.stop());
    EXPECT_FALSE(loop.is_running());
}

}  // namespace
