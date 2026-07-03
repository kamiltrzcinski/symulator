// tests/server/test_dispatch_bus.cpp
//
// Tests for DispatchBus: verifies that make_event_frame() produces correctly
// structured DOMAIN_EVENT wire frames for each DeviceStateChange arm.
// Uses DispatchBus in isolation — no real TransportGateway or network I/O.

#include "engine/core/control_system.hpp"
#include "engine/core/track_model.hpp"
#include "server/db_writer.hpp"
#include "server/dispatch_bus.hpp"
#include "server/frame.hpp"

#include <flatbuffers/flatbuffers.h>
#include "events_generated.h"

#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

#include "server/ownership_guard.hpp"
#include "server/transport_gateway.hpp"

namespace
{

struct BusTestFixture
{
    engine::core::PriorityCommandQueue<engine::core::EnvelopedCommand> cmd_queue;
    server::OwnershipGuard ownership;
    engine::core::AtomicSnapshot snapshot;
    server::TransportGateway gateway{cmd_queue, ownership, snapshot};
    server::NullDbWriter null_db;
    server::DispatchBus bus{gateway, null_db, "00000000-0000-0000-0000-000000000001"};
};

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

// Convenience UID factories
static constexpr UID kSig1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
static constexpr UID kSig2 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 2);
static constexpr UID kSw5 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 5);
static constexpr UID kBlk2 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION, 1, 2);
static constexpr UID kBlk3 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION, 1, 3);
static constexpr UID kBlk1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION, 1, 1);
static constexpr UID kDer1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::DERAILER, 1, 1);
static constexpr UID kRoute = make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 1);
static constexpr UID kSec1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
static constexpr UID kSec2 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 2);
static constexpr UID kAlarm = make_uid(UIDDomain::OPERATIONS, UIDKind::ALARM, 1, 1);

TEST(DispatchBus, SignalAspectChanged)
{
    BusTestFixture f;
    SignalAspectChange ch;
    ch.uid = kSig1;
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
    ch.uid = kSw5;
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
    ch.uid = kBlk2;
    ch.new_state = BlockSectionState::OPEN;

    const auto frame = f.bus.make_event_frame(ch, 1u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x06u);
}

TEST(DispatchBus, TrackSectionOccupancyChanged)
{
    BusTestFixture f;
    TrackSectionOccupancyChange ch;
    ch.uid = kSec1;
    ch.occupancy = TrackOccupancy::OCCUPIED;
    ch.axle_count = 12;
    ch.train_uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST, 0, 1);

    const auto frame = f.bus.make_event_frame(ch, 77u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_TRUE(ev.valid);
    EXPECT_EQ(ev.event_type, 0x04u);  // kTrackSectionOccupancyChanged (doc 09)

    const auto* body = flatbuffers::GetRoot<proto::TrackSectionOccupancyChanged>(ev.fb_body.data());
    ASSERT_NE(body, nullptr);
    EXPECT_EQ(body->uid(), kSec1.value);
    EXPECT_TRUE(body->occupied());
    EXPECT_EQ(body->axle_count(), 12);
    EXPECT_EQ(body->train_uid(), ch.train_uid.value);
}

TEST(DispatchBus, TrackSectionOccupancyChanged_FreeSection)
{
    BusTestFixture f;
    TrackSectionOccupancyChange ch;
    ch.uid = kSec2;
    ch.occupancy = TrackOccupancy::FREE;
    ch.axle_count = 0;

    const auto frame = f.bus.make_event_frame(ch, 78u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x04u);

    const auto* body = flatbuffers::GetRoot<proto::TrackSectionOccupancyChanged>(ev.fb_body.data());
    ASSERT_NE(body, nullptr);
    EXPECT_FALSE(body->occupied());
    EXPECT_EQ(body->axle_count(), 0);
    EXPECT_EQ(body->train_uid(), 0u);
}

TEST(DispatchBus, BlockDirectionChanged)
{
    BusTestFixture f;
    BlockDirectionChange ch;
    ch.uid = kBlk3;
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
    ch.uid = kDer1;
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
    ch.route.uid = kRoute;
    ch.route.from_signal_uid = kSig1;
    ch.route.to_signal_uid = kSig2;
    ch.route.section_uids = {kSec1, kSec2};

    const auto frame = f.bus.make_event_frame(ch, 0u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x07u);
}

TEST(DispatchBus, RouteReleased)
{
    BusTestFixture f;
    RouteRemoved ch;
    ch.route_uid = kRoute;
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
    ch.alarm.uid = kAlarm;
    ch.alarm.kind = "SWITCH_FAILURE";
    ch.alarm.object_uid = kSw5;
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
    ch.alarm_uid = kAlarm;

    const auto frame = f.bus.make_event_frame(ch, 0u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_EQ(ev.event_type, 0x0Bu);
}

TEST(DispatchBus, EventIdMonotonicallyIncreasing)
{
    BusTestFixture f;
    SignalAspectChange ch;
    ch.uid = kSig1;
    ch.new_aspect = SignalAspect::S1_STOP;
    ch.cause = ChangeCause::COMMAND;

    const auto f1 = parse_event_frame(*f.bus.make_event_frame(ch, 0u));
    const auto f2 = parse_event_frame(*f.bus.make_event_frame(ch, 0u));
    EXPECT_GT(f2.event_id, f1.event_id);
}

TEST(DispatchBus, OperatorCommandStateChanged)
{
    BusTestFixture f;
    OperatorCommandStateChange ch;
    ch.uid = kSig1;
    ch.target_kind = OperatorTargetKind::SIGNAL;
    ch.code = OperatorCommandCode::SES;
    ch.active = true;

    const auto frame = f.bus.make_event_frame(ch, 0u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_TRUE(ev.valid);
    EXPECT_EQ(ev.event_type, 0x11u);
}

TEST(DispatchBus, Ml8CommandStateChanged)
{
    BusTestFixture f;
    Ml8CommandStateChange ch;
    ch.uid = kBlk1;
    ch.target_kind = OperatorTargetKind::BLOCK_SECTION;
    ch.code = Ml8CommandCode::AK;
    ch.active = true;

    const auto frame = f.bus.make_event_frame(ch, 0u);
    ASSERT_TRUE(frame.has_value());
    const auto ev = parse_event_frame(*frame);
    EXPECT_TRUE(ev.valid);
    EXPECT_EQ(ev.event_type, 0x12u);
}
