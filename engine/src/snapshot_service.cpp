// engine/src/snapshot_service.cpp

#include "engine/core/snapshot_service.hpp"
#include "engine/core/engine_snapshot.hpp"
#include "engine/core/types.hpp"

// FlatBuffers generated headers
#include "common_generated.h"
#include "snapshot_generated.h"

#include <flatbuffers/flatbuffers.h>

#include <cstdint>
#include <span>
#include <vector>

namespace engine::core
{

namespace
{

// ── Enum converters ───────────────────────────────────────────────────────────
// C++ enum values are defined to match the proto equivalents so a static_cast
// is correct.  Asserted in SnapshotService tests.

proto::Aspect to_proto_aspect(SignalAspect a)
{
    return static_cast<proto::Aspect>(static_cast<uint8_t>(a));
}

proto::SwitchPosition to_proto_switch_pos(SwitchPosition p)
{
    return static_cast<proto::SwitchPosition>(static_cast<uint8_t>(p));
}

proto::DerailerPosition to_proto_derailer_pos(DerailerState s)
{
    return static_cast<proto::DerailerPosition>(static_cast<uint8_t>(s));
}

// NOTE: C++ BlockSectionState is { CLOSED=0, OPEN=1 } but proto has { OPEN=0, CLOSED=1 }.
// Must map explicitly rather than casting by value.
proto::BlockSectionState to_proto_block_state(BlockSectionState s)
{
    return (s == BlockSectionState::CLOSED) ? proto::BlockSectionState_CLOSED
                                            : proto::BlockSectionState_OPEN;
}

proto::BlockDirectionState to_proto_block_dir(BlockDirectionState d)
{
    return static_cast<proto::BlockDirectionState>(static_cast<uint8_t>(d));
}

// Map alarm kind string to the events.fbs AlarmType byte value.
int8_t alarm_kind_byte(const std::string& kind)
{
    if (kind == "TRACK_OCCUPIED_UNEXPECTEDLY")
        return 0;
    if (kind == "SWITCH_POSITION_MISMATCH")
        return 1;
    if (kind == "SIGNAL_OVERRUN")
        return 2;
    if (kind == "DERAILER_CONFLICT")
        return 3;
    if (kind == "COMMS_FAILURE")
        return 4;
    if (kind == "ENGINE_FAULT")
        return 5;
    return 0;
}

}  // anonymous namespace

// ── SnapshotService::serialize ────────────────────────────────────────────────

std::vector<uint8_t> SnapshotService::serialize(const EngineSnapshot& snap)
{
    flatbuffers::FlatBufferBuilder builder(16 * 1024);

    // ── Switches ──────────────────────────────────────────────────────────────
    std::vector<flatbuffers::Offset<proto::SwitchState>> switch_offsets;
    switch_offsets.reserve(snap.switches.size());
    for (const auto& [gid, sw] : snap.switches)
    {
        auto gid_off = builder.CreateString(gid.value);
        proto::SwitchStateBuilder sb(builder);
        sb.add_g_id(gid_off);
        sb.add_position(to_proto_switch_pos(sw.position));
        sb.add_occupied(sw.occupancy == TrackOccupancy::OCCUPIED);
        sb.add_axle_count(sw.axle_count);
        sb.add_locked_by_route(sw.locked_by_route.has_value());
        switch_offsets.push_back(sb.Finish());
    }
    auto switches_vec = builder.CreateVector(switch_offsets);

    // ── Track sections ────────────────────────────────────────────────────────
    std::vector<flatbuffers::Offset<proto::TrackSectionState>> ts_offsets;
    ts_offsets.reserve(snap.track_sections.size());
    for (const auto& [gid, ts] : snap.track_sections)
    {
        auto gid_off = builder.CreateString(gid.value);
        proto::TrackSectionStateBuilder tb(builder);
        tb.add_g_id(gid_off);
        tb.add_occupied(ts.occupancy == TrackOccupancy::OCCUPIED);
        tb.add_axle_count(ts.axle_count);
        ts_offsets.push_back(tb.Finish());
    }
    auto ts_vec = builder.CreateVector(ts_offsets);

    // ── Signals ───────────────────────────────────────────────────────────────
    std::vector<flatbuffers::Offset<proto::SignalState>> sig_offsets;
    sig_offsets.reserve(snap.signals.size());
    for (const auto& [gid, sig] : snap.signals)
    {
        auto gid_off = builder.CreateString(gid.value);
        proto::SignalStateBuilder sigb(builder);
        sigb.add_g_id(gid_off);
        sigb.add_aspect(to_proto_aspect(sig.current_aspect));
        sig_offsets.push_back(sigb.Finish());
    }
    auto sig_vec = builder.CreateVector(sig_offsets);

    // ── Derailers ─────────────────────────────────────────────────────────────
    std::vector<flatbuffers::Offset<proto::DerailerState>> der_offsets;
    der_offsets.reserve(snap.derailers.size());
    for (const auto& [gid, der] : snap.derailers)
    {
        auto gid_off = builder.CreateString(gid.value);
        proto::DerailerStateBuilder db(builder);
        db.add_g_id(gid_off);
        db.add_position(to_proto_derailer_pos(der.state));
        der_offsets.push_back(db.Finish());
    }
    auto der_vec = builder.CreateVector(der_offsets);

    // ── Block sections ────────────────────────────────────────────────────────
    std::vector<flatbuffers::Offset<proto::BlockSectionSnapshotState>> bs_offsets;
    bs_offsets.reserve(snap.block_sections.size());
    for (const auto& [gid, bs] : snap.block_sections)
    {
        auto gid_off = builder.CreateString(gid.value);
        proto::BlockSectionSnapshotStateBuilder bsb(builder);
        bsb.add_g_id(gid_off);
        bsb.add_state(to_proto_block_state(bs.state));
        bsb.add_block_direction(to_proto_block_dir(bs.direction));
        bs_offsets.push_back(bsb.Finish());
    }
    auto bs_vec = builder.CreateVector(bs_offsets);

    // ── Active routes ─────────────────────────────────────────────────────────
    std::vector<flatbuffers::Offset<proto::RouteState>> route_offsets;
    route_offsets.reserve(snap.routes.size());
    for (const auto& [gid, rt] : snap.routes)
    {
        auto rid_off = builder.CreateString(rt.route_id.value);
        auto from_off = builder.CreateString(rt.from_signal_gid.value);
        auto to_off = builder.CreateString(rt.to_signal_gid.value);

        std::vector<flatbuffers::Offset<flatbuffers::String>> sec_strs;
        sec_strs.reserve(rt.section_gids.size());
        for (const auto& s : rt.section_gids)
            sec_strs.push_back(builder.CreateString(s.value));
        auto secs_off = builder.CreateVector(sec_strs);

        proto::RouteStateBuilder rb(builder);
        rb.add_route_id(rid_off);
        rb.add_from_g_id(from_off);
        rb.add_to_g_id(to_off);
        rb.add_section_ids(secs_off);
        route_offsets.push_back(rb.Finish());
    }
    auto route_vec = builder.CreateVector(route_offsets);

    // ── Alarms ────────────────────────────────────────────────────────────────
    std::vector<flatbuffers::Offset<proto::AlarmState>> alarm_offsets;
    alarm_offsets.reserve(snap.alarms.size());
    for (const auto& [gid, al] : snap.alarms)
    {
        auto aid_off = builder.CreateString(al.alarm_id.value);
        auto obj_off = builder.CreateString(al.object_gid.value);
        auto msg_off = builder.CreateString(al.message);
        proto::AlarmStateBuilder ab(builder);
        ab.add_alarm_id(aid_off);
        ab.add_alarm_type(alarm_kind_byte(al.kind));
        ab.add_object_g_id(obj_off);
        ab.add_message(msg_off);
        alarm_offsets.push_back(ab.Finish());
    }
    auto alarm_vec = builder.CreateVector(alarm_offsets);

    // ── Session string ────────────────────────────────────────────────────────
    auto session_off = builder.CreateString(snap.session);

    // ── Root Snapshot table ───────────────────────────────────────────────────
    proto::SnapshotBuilder sb(builder);
    sb.add_schema_version(1);
    sb.add_session_id(session_off);
    sb.add_seq_cursor(static_cast<uint32_t>(snap.tick));
    sb.add_switches(switches_vec);
    sb.add_track_sections(ts_vec);
    sb.add_signals(sig_vec);
    sb.add_derailers(der_vec);
    sb.add_block_sections(bs_vec);
    sb.add_active_routes(route_vec);
    sb.add_active_alarms(alarm_vec);

    builder.Finish(sb.Finish());

    const uint8_t* buf = builder.GetBufferPointer();
    const std::size_t size = builder.GetSize();
    return {buf, buf + size};
}

// ── SnapshotService::chunk ────────────────────────────────────────────────────

std::vector<std::vector<uint8_t>> SnapshotService::chunk(std::span<const uint8_t> binary,
                                                         std::size_t chunk_size)
{
    if (binary.empty() || chunk_size == 0)
        return {{}};

    std::vector<std::vector<uint8_t>> result;
    result.reserve((binary.size() + chunk_size - 1) / chunk_size);

    std::size_t offset = 0;
    while (offset < binary.size())
    {
        const std::size_t len = std::min(chunk_size, binary.size() - offset);
        result.emplace_back(binary.begin() + offset, binary.begin() + offset + len);
        offset += len;
    }
    return result;
}

}  // namespace engine::core
