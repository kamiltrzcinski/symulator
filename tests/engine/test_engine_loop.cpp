#include <gtest/gtest.h>

#include <engine/core/command.hpp>
#include <engine/core/control_system.hpp>
#include <engine/core/engine_loop.hpp>
#include <engine/core/engine_snapshot.hpp>
#include <engine/core/engine_state.hpp>
#include <engine/core/priority_command_queue.hpp>
#include <engine/core/track_model.hpp>
#include <engine/core/types.hpp>

#include <atomic>
#include <chrono>
#include <thread>

namespace
{

using namespace engine::core;

// ── Stub IControlSystem ───────────────────────────────────────────────────────
// Accepts all commands and produces no state changes.

class StubControlSystem final : public IControlSystem
{
public:
    ControlSystemID system_id() const override { return ControlSystemID{"stub"}; }

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
    ControlSystemID system_id() const override { return ControlSystemID{"reject-all"}; }

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
    sw.gid = GID{"ZWR-X"};
    sw.pid = "zwrX";
    sw.sid = SID{"TST"};
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
    cmd.payload = SetSwitchPositionCmd{GID{"ZWR-X"}, SwitchPosition::DIVERGENT};
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
    cmd.payload = SetSignalAspectCmd{GID{"SEM-X"}, SignalAspect::S2_PROCEED};
    q.push(std::move(cmd), CommandPriority::NORMAL);

    EXPECT_TRUE(wait_for([&] { return nak_count.load() >= 1; }, std::chrono::milliseconds{400}));
    loop.stop();
    EXPECT_GE(nak_count.load(), 1);
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
