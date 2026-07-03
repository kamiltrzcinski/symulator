// server/src/dispatch_bus.cpp

#include "server/dispatch_bus.hpp"

#include "engine/core/track_model.hpp"
#include "events_generated.h"
#include "server/frame.hpp"

#include <chrono>
#include <cstring>
#include <string>
#include <variant>

namespace server
{

// event_type constants — see docs/09-communication-contract.md
namespace event_type
{
constexpr uint8_t kSwitchPositionChanged = 0x01;
constexpr uint8_t kSignalAspectChanged = 0x03;
constexpr uint8_t kTrackSectionOccupancyChanged = 0x04;
constexpr uint8_t kDerailerPositionChanged = 0x05;
constexpr uint8_t kBlockSectionStateChanged = 0x06;
constexpr uint8_t kRouteSet = 0x07;
constexpr uint8_t kRouteReleased = 0x08;
// 0x09 = TrainMovement — reserved (doc 09); no emitter yet, see plan_przed_klientem.md Z1.5.
constexpr uint8_t kAlarmRaised = 0x0A;
constexpr uint8_t kAlarmCleared = 0x0B;
constexpr uint8_t kBlockDirectionStateChanged = 0x10;
constexpr uint8_t kOperatorCommandStateChanged = 0x11;
constexpr uint8_t kMl8CommandStateChanged = 0x12;
}  // namespace event_type

// ── Helpers ───────────────────────────────────────────────────────────────────

static uint64_t now_us()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count());
}

static void write_u32_le(uint8_t* dst, uint32_t v)
{
    dst[0] = static_cast<uint8_t>(v & 0xFF);
    dst[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
    dst[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
    dst[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
}

static void write_u64_le(uint8_t* dst, uint64_t v)
{
    for (int i = 0; i < 8; ++i)
        dst[i] = static_cast<uint8_t>((v >> (8 * i)) & 0xFF);
}

static proto::DerailerPosition to_proto_derailer(engine::core::DerailerState s)
{
    return (s == engine::core::DerailerState::LOCKED) ? proto::DerailerPosition_LOCKED
                                                      : proto::DerailerPosition_UNLOCKED;
}

static proto::BlockSectionState to_proto_block_section(engine::core::BlockSectionState s)
{
    return (s == engine::core::BlockSectionState::CLOSED) ? proto::BlockSectionState_CLOSED
                                                          : proto::BlockSectionState_OPEN;
}

static proto::BlockDirectionState to_proto_direction(engine::core::BlockDirectionState s)
{
    return static_cast<proto::BlockDirectionState>(static_cast<int>(s));
}

static proto::ChangeCause to_proto_cause(engine::core::ChangeCause c)
{
    return static_cast<proto::ChangeCause>(static_cast<int>(c));
}

static proto::RouteReleaseReason to_proto_release_reason(const std::string& reason)
{
    if (reason == "OPERATOR_CANCEL")
        return proto::RouteReleaseReason_OPERATOR_CANCEL;
    if (reason == "TIMEOUT")
        return proto::RouteReleaseReason_TIMEOUT;
    if (reason == "CONFLICT")
        return proto::RouteReleaseReason_CONFLICT;
    return proto::RouteReleaseReason_TRAIN_CLEARED;
}

// ── DispatchBus ───────────────────────────────────────────────────────────────

DispatchBus::DispatchBus(TransportGateway& gateway, IDbWriter& db_writer, std::string session_id)
    : gateway_(gateway), db_writer_(db_writer), session_id_(std::move(session_id))
{
}

engine::core::EngineLoop::StateChangesCallback DispatchBus::make_engine_callback()
{
    return [this](const std::vector<engine::core::DeviceStateChange>& changes)
    { on_state_changes(changes); };
}

// static
std::vector<uint8_t> DispatchBus::build_event_prefix(uint8_t event_type, uint32_t event_id,
                                                     uint64_t timestamp_us)
{
    std::vector<uint8_t> prefix(13);
    prefix[0] = event_type;
    write_u32_le(prefix.data() + 1, event_id);
    write_u64_le(prefix.data() + 5, timestamp_us);
    return prefix;
}

std::optional<std::vector<uint8_t>> DispatchBus::make_event_frame(
    const engine::core::DeviceStateChange& change, uint64_t timestamp_us)
{
    const uint32_t eid = next_event_id_.fetch_add(1, std::memory_order_relaxed);

    return std::visit(
        [&](const auto& ev) -> std::optional<std::vector<uint8_t>>
        {
            using T = std::decay_t<decltype(ev)>;

            flatbuffers::FlatBufferBuilder fbb(256);
            uint8_t et = 0;

            if constexpr (std::is_same_v<T, engine::core::SignalAspectChange>)
            {
                et = event_type::kSignalAspectChanged;
                auto off = proto::CreateSignalAspectChanged(
                    fbb, ev.uid.value, static_cast<proto::Aspect>(static_cast<int>(ev.new_aspect)),
                    to_proto_cause(ev.cause));
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::SwitchPositionChange>)
            {
                et = event_type::kSwitchPositionChanged;
                auto off = proto::CreateSwitchPositionChanged(
                    fbb, ev.uid.value,
                    static_cast<proto::SwitchPosition>(static_cast<int>(ev.new_position)),
                    to_proto_cause(ev.cause));
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::SwitchLocked>)
            {
                et = event_type::kSwitchPositionChanged;
                auto off = proto::CreateSwitchPositionChanged(fbb, ev.switch_uid.value,
                                                              proto::SwitchPosition_STRAIGHT,
                                                              proto::ChangeCause_AUTO);
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::SwitchUnlocked>)
            {
                et = event_type::kSwitchPositionChanged;
                auto off = proto::CreateSwitchPositionChanged(fbb, ev.switch_uid.value,
                                                              proto::SwitchPosition_STRAIGHT,
                                                              proto::ChangeCause_AUTO);
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::DerailerStateChange>)
            {
                et = event_type::kDerailerPositionChanged;
                auto off = proto::CreateDerailerPositionChanged(
                    fbb, ev.uid.value, to_proto_derailer(ev.new_state), to_proto_cause(ev.cause));
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::TrackSectionOccupancyChange>)
            {
                et = event_type::kTrackSectionOccupancyChanged;
                auto off = proto::CreateTrackSectionOccupancyChanged(
                    fbb, ev.uid.value, ev.occupancy == engine::core::TrackOccupancy::OCCUPIED,
                    ev.axle_count, ev.train_uid.value);
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::BlockSectionStateChange>)
            {
                et = event_type::kBlockSectionStateChanged;
                auto off = proto::CreateBlockSectionStateChanged(
                    fbb, ev.uid.value, to_proto_block_section(ev.new_state));
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::BlockDirectionChange>)
            {
                et = event_type::kBlockDirectionStateChanged;
                auto off = proto::CreateBlockDirectionStateChanged(
                    fbb, ev.uid.value, to_proto_direction(ev.new_direction),
                    proto::ChangeCause_COMMAND);
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::OperatorCommandStateChange>)
            {
                et = event_type::kOperatorCommandStateChanged;
                auto off = proto::CreateOperatorCommandStateChanged(
                    fbb, ev.uid.value,
                    static_cast<proto::OperatorTargetKind>(static_cast<int>(ev.target_kind)),
                    static_cast<proto::OperatorCommandCode>(static_cast<int>(ev.code)), ev.active);
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::Ml8CommandStateChange>)
            {
                et = event_type::kMl8CommandStateChanged;
                auto off = proto::CreateMl8CommandStateChanged(
                    fbb, ev.uid.value,
                    static_cast<proto::OperatorTargetKind>(static_cast<int>(ev.target_kind)),
                    static_cast<proto::Ml8CommandCode>(static_cast<int>(ev.code)), ev.active);
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::RouteAdded>)
            {
                et = event_type::kRouteSet;
                const auto& r = ev.route;
                std::vector<uint64_t> sec_uids;
                sec_uids.reserve(r.section_uids.size());
                for (const auto& s : r.section_uids)
                    sec_uids.push_back(s.value);
                auto sec_vec = fbb.CreateVector(sec_uids);
                auto off = proto::CreateRouteSet(fbb, r.uid.value, r.from_signal_uid.value,
                                                 r.to_signal_uid.value, sec_vec);
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::RouteRemoved>)
            {
                et = event_type::kRouteReleased;
                auto off = proto::CreateRouteReleased(fbb, ev.route_uid.value,
                                                      to_proto_release_reason(ev.reason));
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::AlarmRaised>)
            {
                et = event_type::kAlarmRaised;
                const auto& a = ev.alarm;
                auto message_off = fbb.CreateString(a.message);
                proto::AlarmType alarm_type = proto::AlarmType_TRACK_OCCUPIED_UNEXPECTEDLY;
                if (a.kind == "SWITCH_FAILURE")
                    alarm_type = proto::AlarmType_SWITCH_POSITION_MISMATCH;
                else if (a.kind == "SIGNAL_FAILURE")
                    alarm_type = proto::AlarmType_SIGNAL_OVERRUN;
                else if (a.kind == "DERAILER_CONFLICT")
                    alarm_type = proto::AlarmType_DERAILER_CONFLICT;
                else if (a.kind == "COMMS_FAILURE")
                    alarm_type = proto::AlarmType_COMMS_FAILURE;
                else if (a.kind == "ENGINE_FAULT")
                    alarm_type = proto::AlarmType_ENGINE_FAULT;
                auto off = proto::CreateAlarmRaised(fbb, a.uid.value, alarm_type,
                                                    a.object_uid.value, message_off);
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::AlarmCleared>)
            {
                et = event_type::kAlarmCleared;
                flatbuffers::FlatBufferBuilder fbb2(64);
                auto off = proto::CreateAlarmCleared(fbb2, ev.alarm_uid.value);
                fbb2.Finish(off);
                const auto prefix = build_event_prefix(et, eid, timestamp_us);
                std::vector<uint8_t> payload;
                payload.reserve(prefix.size() + fbb2.GetSize());
                payload.insert(payload.end(), prefix.begin(), prefix.end());
                payload.insert(payload.end(), fbb2.GetBufferPointer(),
                               fbb2.GetBufferPointer() + fbb2.GetSize());
                const uint32_t seq = tx_seq_.fetch_add(1, std::memory_order_relaxed);
                return encode_frame(msg_type::kDomainEvent, 0, seq, payload);
            }
            else
            {
                return std::nullopt;
            }

            const auto prefix = build_event_prefix(et, eid, timestamp_us);
            std::vector<uint8_t> payload;
            payload.reserve(prefix.size() + fbb.GetSize());
            payload.insert(payload.end(), prefix.begin(), prefix.end());
            payload.insert(payload.end(), fbb.GetBufferPointer(),
                           fbb.GetBufferPointer() + fbb.GetSize());
            const uint32_t seq = tx_seq_.fetch_add(1, std::memory_order_relaxed);
            return encode_frame(msg_type::kDomainEvent, 0, seq, payload);
        },
        change);
}

void DispatchBus::on_state_changes(const std::vector<engine::core::DeviceStateChange>& changes)
{
    const uint64_t ts = now_us();
    for (const auto& change : changes)
    {
        auto frame = make_event_frame(change, ts);
        if (!frame)
            continue;

        gateway_.broadcast(*frame);

        if (frame->size() > 29)
        {
            DomainEventRow rec;
            rec.event_type = (*frame)[16];
            rec.event_id = static_cast<uint32_t>((*frame)[17]) |
                           (static_cast<uint32_t>((*frame)[18]) << 8) |
                           (static_cast<uint32_t>((*frame)[19]) << 16) |
                           (static_cast<uint32_t>((*frame)[20]) << 24);
            for (int i = 0; i < 8; ++i)
                rec.timestamp_us |= static_cast<uint64_t>((*frame)[21 + i]) << (8 * i);
            rec.object_uid = object_uid_from_change(change);
            rec.payload.assign(frame->begin() + 29, frame->end());
            db_writer_.write_domain_event(session_id_, std::move(rec));
        }
    }
}

// static
std::optional<std::uint64_t> DispatchBus::object_uid_from_change(
    const engine::core::DeviceStateChange& change)
{
    using namespace engine::core;
    return std::visit(
        [](const auto& ev) -> std::optional<std::uint64_t>
        {
            using T = std::decay_t<decltype(ev)>;
            if constexpr (requires { ev.uid; })
                return ev.uid.value;
            else if constexpr (requires { ev.switch_uid; })
                return ev.switch_uid.value;
            else if constexpr (std::is_same_v<T, RouteAdded>)
                return ev.route.uid.value;
            else if constexpr (std::is_same_v<T, RouteRemoved>)
                return ev.route_uid.value;
            else if constexpr (std::is_same_v<T, AlarmRaised>)
                return ev.alarm.object_uid.value;
            else if constexpr (std::is_same_v<T, AlarmCleared>)
                return ev.alarm_uid.value;
            else
                return std::nullopt;
        },
        change);
}

}  // namespace server
