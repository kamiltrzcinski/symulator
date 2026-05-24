// server/src/dispatch_bus.cpp

#include "server/dispatch_bus.hpp"

#include "engine/core/track_model.hpp"
#include "events_generated.h"
#include "server/frame.hpp"

#include <chrono>
#include <cstring>
#include <variant>

namespace server
{

// event_type constants — see docs/09-communication-contract.md
namespace event_type
{
constexpr uint8_t kSwitchPositionChanged = 0x01;
constexpr uint8_t kSignalAspectChanged = 0x03;
constexpr uint8_t kDerailerPositionChanged = 0x05;
constexpr uint8_t kBlockSectionStateChanged = 0x06;
constexpr uint8_t kRouteSet = 0x07;
constexpr uint8_t kRouteReleased = 0x08;
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

// Write a uint32_t little-endian into a buffer.
static void write_u32_le(uint8_t* dst, uint32_t v)
{
    dst[0] = static_cast<uint8_t>(v & 0xFF);
    dst[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
    dst[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
    dst[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
}

// Write a uint64_t little-endian into a buffer.
static void write_u64_le(uint8_t* dst, uint64_t v)
{
    for (int i = 0; i < 8; ++i)
        dst[i] = static_cast<uint8_t>((v >> (8 * i)) & 0xFF);
}

// Map engine DerailerState → proto DerailerPosition
static proto::DerailerPosition to_proto_derailer(engine::core::DerailerState s)
{
    switch (s)
    {
        case engine::core::DerailerState::LOCKED:
            return proto::DerailerPosition_LOCKED;
        case engine::core::DerailerState::UNLOCKED:
            return proto::DerailerPosition_UNLOCKED;
    }
    return proto::DerailerPosition_LOCKED;
}

// Map engine BlockSectionState → proto BlockSectionState
// INVERTED: engine CLOSED=0/OPEN=1, proto OPEN=0/CLOSED=1
static proto::BlockSectionState to_proto_block_section(engine::core::BlockSectionState s)
{
    if (s == engine::core::BlockSectionState::CLOSED)
        return proto::BlockSectionState_CLOSED;
    return proto::BlockSectionState_OPEN;
}

// Map engine BlockDirectionState → proto BlockDirectionState (same ordinals)
static proto::BlockDirectionState to_proto_direction(engine::core::BlockDirectionState s)
{
    return static_cast<proto::BlockDirectionState>(static_cast<int>(s));
}

// Map engine ChangeCause → proto ChangeCause (same ordinals)
static proto::ChangeCause to_proto_cause(engine::core::ChangeCause c)
{
    return static_cast<proto::ChangeCause>(static_cast<int>(c));
}

// Map RouteRemoved::reason string → proto RouteReleaseReason
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

            // ── Signal aspect ─────────────────────────────────────────────────
            if constexpr (std::is_same_v<T, engine::core::SignalAspectChange>)
            {
                et = event_type::kSignalAspectChanged;
                auto gid_off = fbb.CreateString(ev.gid.value);
                auto off = proto::CreateSignalAspectChanged(
                    fbb, gid_off, static_cast<proto::Aspect>(static_cast<int>(ev.new_aspect)),
                    to_proto_cause(ev.cause));
                fbb.Finish(off);
            }
            // ── Switch position ───────────────────────────────────────────────
            else if constexpr (std::is_same_v<T, engine::core::SwitchPositionChange>)
            {
                et = event_type::kSwitchPositionChanged;
                auto gid_off = fbb.CreateString(ev.gid.value);
                auto off = proto::CreateSwitchPositionChanged(
                    fbb, gid_off,
                    static_cast<proto::SwitchPosition>(static_cast<int>(ev.new_position)),
                    to_proto_cause(ev.cause));
                fbb.Finish(off);
            }
            // ── Switch locked / unlocked: emit SwitchPositionChanged ──────────
            else if constexpr (std::is_same_v<T, engine::core::SwitchLocked>)
            {
                et = event_type::kSwitchPositionChanged;
                auto gid_off = fbb.CreateString(ev.switch_gid.value);
                // Position unchanged — we still emit to signal the lock; use default (STRAIGHT)
                auto off = proto::CreateSwitchPositionChanged(
                    fbb, gid_off, proto::SwitchPosition_STRAIGHT, proto::ChangeCause_AUTO);
                fbb.Finish(off);
            }
            else if constexpr (std::is_same_v<T, engine::core::SwitchUnlocked>)
            {
                et = event_type::kSwitchPositionChanged;
                auto gid_off = fbb.CreateString(ev.switch_gid.value);
                auto off = proto::CreateSwitchPositionChanged(
                    fbb, gid_off, proto::SwitchPosition_STRAIGHT, proto::ChangeCause_AUTO);
                fbb.Finish(off);
            }
            // ── Derailer ──────────────────────────────────────────────────────
            else if constexpr (std::is_same_v<T, engine::core::DerailerStateChange>)
            {
                et = event_type::kDerailerPositionChanged;
                auto gid_off = fbb.CreateString(ev.gid.value);
                auto off = proto::CreateDerailerPositionChanged(
                    fbb, gid_off, to_proto_derailer(ev.new_state), to_proto_cause(ev.cause));
                fbb.Finish(off);
            }
            // ── Block section ─────────────────────────────────────────────────
            else if constexpr (std::is_same_v<T, engine::core::BlockSectionStateChange>)
            {
                et = event_type::kBlockSectionStateChanged;
                auto gid_off = fbb.CreateString(ev.gid.value);
                auto off = proto::CreateBlockSectionStateChanged(
                    fbb, gid_off, to_proto_block_section(ev.new_state));
                fbb.Finish(off);
            }
            // ── Block direction ───────────────────────────────────────────────
            else if constexpr (std::is_same_v<T, engine::core::BlockDirectionChange>)
            {
                et = event_type::kBlockDirectionStateChanged;
                auto gid_off = fbb.CreateString(ev.gid.value);
                auto off = proto::CreateBlockDirectionStateChanged(
                    fbb, gid_off, to_proto_direction(ev.new_direction), proto::ChangeCause_COMMAND);
                fbb.Finish(off);
            }
            // ── Operator command state ───────────────────────────────────────
            else if constexpr (std::is_same_v<T, engine::core::OperatorCommandStateChange>)
            {
                et = event_type::kOperatorCommandStateChanged;
                auto gid_off = fbb.CreateString(ev.gid.value);
                auto off = proto::CreateOperatorCommandStateChanged(
                    fbb, gid_off,
                    static_cast<proto::OperatorTargetKind>(static_cast<int>(ev.target_kind)),
                    static_cast<proto::OperatorCommandCode>(static_cast<int>(ev.code)), ev.active);
                fbb.Finish(off);
            }
            // ── ML8 command state ────────────────────────────────────────────────
            else if constexpr (std::is_same_v<T, engine::core::Ml8CommandStateChange>)
            {
                et = event_type::kMl8CommandStateChanged;
                auto gid_off = fbb.CreateString(ev.gid.value);
                auto off = proto::CreateMl8CommandStateChanged(
                    fbb, gid_off,
                    static_cast<proto::OperatorTargetKind>(static_cast<int>(ev.target_kind)),
                    static_cast<proto::Ml8CommandCode>(static_cast<int>(ev.code)), ev.active);
                fbb.Finish(off);
            }
            // ── Route set ───────────────────────────────────────────────────
            else if constexpr (std::is_same_v<T, engine::core::RouteAdded>)
            {
                et = event_type::kRouteSet;
                const auto& r = ev.route;
                auto route_id_off = fbb.CreateString(r.route_id.value);
                auto from_g_id_off = fbb.CreateString(r.from_signal_gid.value);
                auto to_g_id_off = fbb.CreateString(r.to_signal_gid.value);
                // Build vector of section strings
                std::vector<flatbuffers::Offset<flatbuffers::String>> sec_offs;
                sec_offs.reserve(r.section_gids.size());
                for (const auto& s : r.section_gids)
                    sec_offs.push_back(fbb.CreateString(s.value));
                auto sec_vec_off = fbb.CreateVector(sec_offs);
                auto off = proto::CreateRouteSet(fbb, route_id_off, from_g_id_off, to_g_id_off,
                                                 sec_vec_off);
                fbb.Finish(off);
            }
            // ── Route released ────────────────────────────────────────────────
            else if constexpr (std::is_same_v<T, engine::core::RouteRemoved>)
            {
                et = event_type::kRouteReleased;
                auto route_id_off = fbb.CreateString(ev.route_id.value);
                auto off = proto::CreateRouteReleased(fbb, route_id_off,
                                                      to_proto_release_reason(ev.reason));
                fbb.Finish(off);
            }
            // ── Alarm raised ──────────────────────────────────────────────────
            else if constexpr (std::is_same_v<T, engine::core::AlarmRaised>)
            {
                et = event_type::kAlarmRaised;
                const auto& a = ev.alarm;
                auto alarm_id_off = fbb.CreateString(a.alarm_id.value);
                auto obj_g_id_off = fbb.CreateString(a.object_gid.value);
                auto message_off = fbb.CreateString(a.message);
                // AlarmType defaults to TRACK_OCCUPIED_UNEXPECTEDLY if kind unknown
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
                auto off = proto::CreateAlarmRaised(fbb, alarm_id_off, alarm_type, obj_g_id_off,
                                                    message_off);
                fbb.Finish(off);
            }
            // ── Alarm cleared ─────────────────────────────────────────────────
            else if constexpr (std::is_same_v<T, engine::core::AlarmCleared>)
            {
                et = event_type::kAlarmCleared;
                flatbuffers::FlatBufferBuilder fbb2(64);
                auto alarm_id_off = fbb2.CreateString(ev.alarm_id.value);
                auto off = proto::CreateAlarmCleared(fbb2, alarm_id_off);
                fbb2.Finish(off);
                // Use fbb2 instead
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

            // Common path (all except AlarmCleared which returns early)
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

        // ── Persist to session.events ─────────────────────────────────────────
        // The wire frame layout (from frame.hpp):
        //   [0..15]  transport header (16 bytes)
        //   [16]     event_type
        //   [17..20] event_id  (uint32 LE)
        //   [21..28] timestamp_us (uint64 LE)
        //   [29..]   FlatBuffers body
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
            rec.object_gid = object_gid_from_change(change);
            rec.payload.assign(frame->begin() + 29, frame->end());
            db_writer_.write_domain_event(session_id_, std::move(rec));
        }
    }
}

// ── object_gid_from_change ────────────────────────────────────────────────────

// static
std::optional<std::string> DispatchBus::object_gid_from_change(
    const engine::core::DeviceStateChange& change)
{
    using namespace engine::core;
    return std::visit(
        [](const auto& ev) -> std::optional<std::string>
        {
            using T = std::decay_t<decltype(ev)>;
            // Most variants carry a .gid field (the affected device's GID).
            if constexpr (requires { ev.gid; })
                return ev.gid.value;
            // SwitchLocked / SwitchUnlocked carry .switch_gid instead.
            else if constexpr (requires { ev.switch_gid; })
                return ev.switch_gid.value;
            // Route events: use the route_id as the object identifier.
            else if constexpr (std::is_same_v<T, RouteAdded>)
                return ev.route.route_id.value;
            else if constexpr (std::is_same_v<T, RouteRemoved>)
                return ev.route_id.value;
            // Alarm events.
            else if constexpr (std::is_same_v<T, AlarmRaised>)
                return ev.alarm.object_gid.value;
            else if constexpr (std::is_same_v<T, AlarmCleared>)
                return ev.alarm_id.value;
            else
                return std::nullopt;
        },
        change);
}

}  // namespace server
