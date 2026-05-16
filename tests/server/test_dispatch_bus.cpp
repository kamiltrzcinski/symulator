// tests/server/test_dispatch_bus.cpp
//
// Tests for DispatchBus: verifies that make_event_frame() produces correctly
// structured DOMAIN_EVENT wire frames for each DeviceStateChange arm.
// Uses DispatchBus in isolation — no real TransportGateway or network I/O.

#include "engine/core/control_system.hpp"
#include "engine/core/track_model.hpp"
#include "server/dispatch_bus.hpp"
#include "server/frame.hpp"

#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

// We need a TransportGateway reference in DispatchBus ctor but we never call
// broadcast in these tests, so we use a stub trick: create a bus via inheritance
// or just test make_event_frame directly which only uses internal state.
//
// The cleanest approach: subclass DispatchBus and override broadcast, or simply
// build a minimal real TransportGateway (no start() called).

#include "server/ownership_guard.hpp"
#include "server/transport_gateway.hpp"

namespace
{

// Minimal stub: PriorityCommandQueue requires no_discard — just make it die at queue pop.
struct BusTestFixture
{
    engine::core::PriorityCommandQueue<engine::core::EnvelopedCommand> cmd_queue;
    server::OwnershipGuard ownership;
    engine::core::AtomicSnapshot snapshot;
    server::TransportGateway gateway{cmd_queue, ownership, snapshot};
    server::DispatchBus bus{gateway};
};

// Decode a DOMAIN_EVENT frame back and check prefix.
struct ParsedEvent
{
    bool valid = false;
    uint8_t event_type = 0;
    uint32_t event_id = 0;
    uint64_t ts_us = 0;
    std::vector<uint8_t> fb_body;
};

static ParsedEvent parse_event_frame(const std::vector<uint8_t>& wire)
{
    using namespace server;
    const auto result = decode_frame(wire.data(), wire.size());
    if (result.status != FrameDecodeStatus::kOk)
        return {};
    if (result.frame.msg_type != msg_type::kDomainEvent)
        return {};
    const auto& payload = result.frame.payload;
    if (payload.size() < 13)
        return {};

    ParsedEvent ev;
    ev.valid = true;
    ev.event_type = payload[0];
    ev.event_id = static_cast<uint32_t>(payload[1]) | (static_cast<uint32_t>(payload[2]) << 8) |
                  (static_cast<uint32_t>(payload[3]) << 16) |
                  (static_cast<uint32_t>(payload[4]) << 24);
    for (int i = 0; i < 8; ++i)
        ev.ts_us |= static_cast<uint64_t>(payload[5 + i]) << (8 * i);
    ev.fb_body = {payload.begin() + 13, payload.end()};
    return ev;
}

}  // namespace

using namespace engine::core;

TEST(DispatchBus, SignalAspectChanged)
{
    BusTestFixture f;
    SignalAspectChange ch;
    ch.gid = GID{"SIG-01"};
    ch.new_aspect = SignalAspect::S2_PROCEED;
    ch.cause = ChangeCause::COMMAND;

    const auto frame = f.bus.make_event_frame(ch, 12345u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_TRUE(ev.valid);
    EXPECT_EQ(ev.event_type, 0x03u);  // kSignalAspectChanged
    EXPECT_EQ(ev.ts_us, 12345u);
    EXPECT_FALSE(ev.fb_body.empty());
}

TEST(DispatchBus, SwitchPositionChanged)
{
    BusTestFixture f;
    SwitchPositionChange ch;
    ch.gid = GID{"SW-05"};
    ch.new_position = SwitchPosition::DIVERGENT;
    ch.cause = ChangeCause::COMMAND;

    const auto frame = f.bus.make_event_frame(ch, 42u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x01u);
}

TEST(DispatchBus, BlockSectionStateChanged)
{
    BusTestFixture f;
    BlockSectionStateChange ch;
    ch.gid = GID{"BLK-02"};
    ch.new_state = BlockSectionState::OPEN;

    const auto frame = f.bus.make_event_frame(ch, 1u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x06u);
}

TEST(DispatchBus, BlockDirectionChanged)
{
    BusTestFixture f;
    BlockDirectionChange ch;
    ch.gid = GID{"BLK-03"};
    ch.new_direction = BlockDirectionState::OUTBOUND;

    const auto frame = f.bus.make_event_frame(ch, 0u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x10u);
}

TEST(DispatchBus, DerailerStateChanged)
{
    BusTestFixture f;
    DerailerStateChange ch;
    ch.gid = GID{"DR-01"};
    ch.new_state = DerailerState::UNLOCKED;
    ch.cause = ChangeCause::COMMAND;

    const auto frame = f.bus.make_event_frame(ch, 0u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x05u);
}

TEST(DispatchBus, RouteSet)
{
    BusTestFixture f;
    RouteAdded ch;
    ch.route.route_id = GID{"RTE-01"};
    ch.route.from_signal_gid = GID{"SIG-A"};
    ch.route.to_signal_gid = GID{"SIG-B"};
    ch.route.section_gids = {GID{"SEC-1"}, GID{"SEC-2"}};

    const auto frame = f.bus.make_event_frame(ch, 0u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x07u);
}

TEST(DispatchBus, RouteReleased)
{
    BusTestFixture f;
    RouteRemoved ch;
    ch.route_id = GID{"RTE-01"};
    ch.reason = "TRAIN_CLEARED";

    const auto frame = f.bus.make_event_frame(ch, 0u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x08u);
}

TEST(DispatchBus, AlarmRaised)
{
    BusTestFixture f;
    engine::core::AlarmRaised ch;
    ch.alarm.alarm_id = GID{"ALM-01"};
    ch.alarm.kind = "SWITCH_FAILURE";
    ch.alarm.object_gid = GID{"SW-03"};
    ch.alarm.message = "Switch stuck";

    const auto frame = f.bus.make_event_frame(ch, 0u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x0Au);
}

TEST(DispatchBus, AlarmCleared)
{
    BusTestFixture f;
    engine::core::AlarmCleared ch;
    ch.alarm_id = GID{"ALM-01"};

    const auto frame = f.bus.make_event_frame(ch, 0u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x0Bu);
}

TEST(DispatchBus, EventIdMonotonicallyIncreasing)
{
    BusTestFixture f;
    SignalAspectChange ch;
    ch.gid = GID{"SIG-01"};
    ch.new_aspect = SignalAspect::S1_STOP;
    ch.cause = ChangeCause::COMMAND;

    const auto f1 = parse_event_frame(*f.bus.make_event_frame(ch, 0u));
    const auto f2 = parse_event_frame(*f.bus.make_event_frame(ch, 0u));
    EXPECT_GT(f2.event_id, f1.event_id);
}
