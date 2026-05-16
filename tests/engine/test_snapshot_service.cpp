#include <gtest/gtest.h>

#include <engine/core/engine_snapshot.hpp>
#include <engine/core/snapshot_service.hpp>
#include <engine/core/track_model.hpp>
#include <engine/core/types.hpp>

// FlatBuffers generated headers (to verify the deserialized output)
#include "common_generated.h"
#include "snapshot_generated.h"

#include <flatbuffers/flatbuffers.h>

#include <cstdint>
#include <span>

namespace
{

using namespace engine::core;

// Build a populated EngineSnapshot for testing.
EngineSnapshot make_snapshot()
{
    EngineSnapshot snap;
    snap.session = "SVC_TEST";
    snap.tick = 42;

    // Signal: S1_STOP aspect
    Signal sig;
    sig.gid = GID{"SEM-X"};
    sig.pid = "Wx";
    sig.current_aspect = SignalAspect::S2_PROCEED;
    snap.signals[sig.gid] = sig;

    // Switch: DIVERGENT, locked
    Switch sw;
    sw.gid = GID{"ZWR-Y"};
    sw.pid = "zy";
    sw.position = SwitchPosition::DIVERGENT;
    sw.locked_by_route = GID{"RT-1"};
    sw.occupancy = TrackOccupancy::FREE;
    snap.switches[sw.gid] = sw;

    // Track section: occupied
    TrackSection ts;
    ts.gid = GID{"OT-Z"};
    ts.pid = "tz";
    ts.occupancy = TrackOccupancy::OCCUPIED;
    ts.axle_count = 4;
    snap.track_sections[ts.gid] = ts;

    // Derailer: UNLOCKED
    Derailer der;
    der.gid = GID{"WK-1"};
    der.pid = "wk1";
    der.state = DerailerState::UNLOCKED;
    snap.derailers[der.gid] = der;

    // Block section: CLOSED, OUTBOUND direction
    BlockSection bs;
    bs.gid = GID{"BL-1"};
    bs.pid = "bl1";
    bs.state = BlockSectionState::CLOSED;
    bs.direction = BlockDirectionState::OUTBOUND;
    snap.block_sections[bs.gid] = bs;

    // Active route
    RouteState rt;
    rt.route_id = GID{"RT-1"};
    rt.from_signal_gid = GID{"SEM-X"};
    rt.to_signal_gid = GID{"SEM-Y"};
    rt.section_gids = {GID{"OT-Z"}};
    snap.routes[rt.route_id] = rt;

    // Active alarm
    AlarmState al;
    al.alarm_id = GID{"ALM-1"};
    al.kind = "SWITCH_POSITION_MISMATCH";
    al.object_gid = GID{"ZWR-Y"};
    al.message = "Switch disagreement";
    snap.alarms[al.alarm_id] = al;

    return snap;
}

// ── Serialization ─────────────────────────────────────────────────────────────

TEST(SnapshotService, SerializeProducesBinary)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);
    EXPECT_GT(bin.size(), 0u);
}

TEST(SnapshotService, SerializationIsValidFlatBuffer)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);

    flatbuffers::Verifier verifier(bin.data(), bin.size());
    EXPECT_TRUE(proto::VerifySnapshotBuffer(verifier));
}

TEST(SnapshotService, SessionIdRoundTrip)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);

    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed, nullptr);
    ASSERT_NE(parsed->session_id(), nullptr);
    EXPECT_EQ(parsed->session_id()->str(), "SVC_TEST");
}

TEST(SnapshotService, TickStoredInSeqCursor)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);

    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed, nullptr);
    EXPECT_EQ(parsed->seq_cursor(), 42u);
}

TEST(SnapshotService, SignalAspectRoundTrip)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);

    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed->signals(), nullptr);
    ASSERT_EQ(parsed->signals()->size(), 1u);
    const auto* sig = parsed->signals()->Get(0);
    EXPECT_EQ(sig->g_id()->str(), "SEM-X");
    EXPECT_EQ(sig->aspect(), proto::Aspect_S2_PROCEED);
}

TEST(SnapshotService, SwitchPositionAndLockRoundTrip)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);

    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed->switches(), nullptr);
    ASSERT_EQ(parsed->switches()->size(), 1u);
    const auto* sw = parsed->switches()->Get(0);
    EXPECT_EQ(sw->g_id()->str(), "ZWR-Y");
    EXPECT_EQ(sw->position(), proto::SwitchPosition_DIVERGENT);
    EXPECT_TRUE(sw->locked_by_route());
    EXPECT_FALSE(sw->occupied());
}

TEST(SnapshotService, TrackSectionOccupancyRoundTrip)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);

    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed->track_sections(), nullptr);
    ASSERT_EQ(parsed->track_sections()->size(), 1u);
    const auto* ts = parsed->track_sections()->Get(0);
    EXPECT_EQ(ts->g_id()->str(), "OT-Z");
    EXPECT_TRUE(ts->occupied());
    EXPECT_EQ(ts->axle_count(), 4);
}

TEST(SnapshotService, DerailerPositionRoundTrip)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);

    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed->derailers(), nullptr);
    ASSERT_EQ(parsed->derailers()->size(), 1u);
    const auto* der = parsed->derailers()->Get(0);
    EXPECT_EQ(der->position(), proto::DerailerPosition_UNLOCKED);
}

TEST(SnapshotService, BlockSectionStateAndDirectionRoundTrip)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);

    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed->block_sections(), nullptr);
    ASSERT_EQ(parsed->block_sections()->size(), 1u);
    const auto* bs = parsed->block_sections()->Get(0);
    EXPECT_EQ(bs->state(), proto::BlockSectionState_CLOSED);
    EXPECT_EQ(bs->block_direction(), proto::BlockDirectionState_OUTBOUND);
}

// C++ BlockSectionState has CLOSED=0,OPEN=1 — inverted from proto.
TEST(SnapshotService, BlockSectionStateEnumMappingIsExplicit)
{
    EngineSnapshot snap;
    snap.session = "MAP";

    BlockSection open_bs;
    open_bs.gid = GID{"BL-OPEN"};
    open_bs.pid = "blo";
    open_bs.state = BlockSectionState::OPEN;  // C++ == 1
    open_bs.direction = BlockDirectionState::NEUTRAL;
    snap.block_sections[open_bs.gid] = open_bs;

    const auto bin = SnapshotService::serialize(snap);
    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed->block_sections(), nullptr);
    const auto* bs = parsed->block_sections()->Get(0);
    EXPECT_EQ(bs->state(), proto::BlockSectionState_OPEN);  // proto OPEN == 0
}

TEST(SnapshotService, RouteRoundTrip)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);

    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed->active_routes(), nullptr);
    ASSERT_EQ(parsed->active_routes()->size(), 1u);
    const auto* rt = parsed->active_routes()->Get(0);
    EXPECT_EQ(rt->route_id()->str(), "RT-1");
    EXPECT_EQ(rt->from_g_id()->str(), "SEM-X");
    EXPECT_EQ(rt->to_g_id()->str(), "SEM-Y");
    ASSERT_NE(rt->section_ids(), nullptr);
    ASSERT_EQ(rt->section_ids()->size(), 1u);
    EXPECT_EQ(rt->section_ids()->Get(0)->str(), "OT-Z");
}

TEST(SnapshotService, AlarmRoundTrip)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);

    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed->active_alarms(), nullptr);
    ASSERT_EQ(parsed->active_alarms()->size(), 1u);
    const auto* al = parsed->active_alarms()->Get(0);
    EXPECT_EQ(al->alarm_id()->str(), "ALM-1");
    EXPECT_EQ(al->alarm_type(), 1);  // SWITCH_POSITION_MISMATCH = 1
    EXPECT_EQ(al->message()->str(), "Switch disagreement");
}

TEST(SnapshotService, EmptySnapshotIsValid)
{
    EngineSnapshot empty;
    empty.session = "EMPTY";
    empty.tick = 0;
    const auto bin = SnapshotService::serialize(empty);
    flatbuffers::Verifier verifier(bin.data(), bin.size());
    EXPECT_TRUE(proto::VerifySnapshotBuffer(verifier));
}

// ── Chunking ──────────────────────────────────────────────────────────────────

TEST(SnapshotService, ChunkSmallBinaryProducesOneChunk)
{
    std::vector<uint8_t> binary(100, 0xAB);
    const auto chunks = SnapshotService::chunk(binary, 1024);
    ASSERT_EQ(chunks.size(), 1u);
    EXPECT_EQ(chunks[0].size(), 100u);
}

TEST(SnapshotService, ChunkSplitsAtBoundary)
{
    std::vector<uint8_t> binary(300, 0x01);
    const auto chunks = SnapshotService::chunk(binary, 100);
    EXPECT_EQ(chunks.size(), 3u);
    for (const auto& c : chunks)
        EXPECT_EQ(c.size(), 100u);
}

TEST(SnapshotService, ChunkLastPieceIsSmaller)
{
    std::vector<uint8_t> binary(250, 0x02);
    const auto chunks = SnapshotService::chunk(binary, 100);
    ASSERT_EQ(chunks.size(), 3u);
    EXPECT_EQ(chunks[0].size(), 100u);
    EXPECT_EQ(chunks[1].size(), 100u);
    EXPECT_EQ(chunks[2].size(), 50u);
}

TEST(SnapshotService, ChunkPreservesData)
{
    std::vector<uint8_t> binary(200);
    for (std::size_t i = 0; i < binary.size(); ++i)
        binary[i] = static_cast<uint8_t>(i % 256);

    const auto chunks = SnapshotService::chunk(binary, 80);
    // Reassemble
    std::vector<uint8_t> reassembled;
    for (const auto& c : chunks)
        reassembled.insert(reassembled.end(), c.begin(), c.end());

    EXPECT_EQ(reassembled, binary);
}

TEST(SnapshotService, ChunkRealSnapshotFitsInDefaultChunks)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);
    const auto chunks = SnapshotService::chunk(bin);
    // Each chunk must be ≤ 64 KB
    for (const auto& c : chunks)
        EXPECT_LE(c.size(), SnapshotService::DEFAULT_CHUNK_SIZE);
    // Verify reassembly still parses
    std::vector<uint8_t> reassembled;
    for (const auto& c : chunks)
        reassembled.insert(reassembled.end(), c.begin(), c.end());
    flatbuffers::Verifier verifier(reassembled.data(), reassembled.size());
    EXPECT_TRUE(proto::VerifySnapshotBuffer(verifier));
}

// ── Enum value alignment ──────────────────────────────────────────────────────

TEST(SnapshotService, SignalAspectEnumValuesAligned)
{
    EXPECT_EQ(static_cast<uint8_t>(SignalAspect::S1_STOP), proto::Aspect_S1_STOP);
    EXPECT_EQ(static_cast<uint8_t>(SignalAspect::S2_PROCEED), proto::Aspect_S2_PROCEED);
    EXPECT_EQ(static_cast<uint8_t>(SignalAspect::MS1_STOP), proto::Aspect_MS1_STOP);
    EXPECT_EQ(static_cast<uint8_t>(SignalAspect::MS2_SHUNTING_ALLOWED),
              proto::Aspect_MS2_SHUNTING_ALLOWED);
}

TEST(SnapshotService, SwitchPositionEnumValuesAligned)
{
    EXPECT_EQ(static_cast<uint8_t>(SwitchPosition::STRAIGHT), proto::SwitchPosition_STRAIGHT);
    EXPECT_EQ(static_cast<uint8_t>(SwitchPosition::DIVERGENT), proto::SwitchPosition_DIVERGENT);
    EXPECT_EQ(static_cast<uint8_t>(SwitchPosition::MOVING), proto::SwitchPosition_MOVING);
}

TEST(SnapshotService, BlockDirectionEnumValuesAligned)
{
    EXPECT_EQ(static_cast<uint8_t>(BlockDirectionState::NEUTRAL),
              proto::BlockDirectionState_NEUTRAL);
    EXPECT_EQ(static_cast<uint8_t>(BlockDirectionState::OUTBOUND),
              proto::BlockDirectionState_OUTBOUND);
    EXPECT_EQ(static_cast<uint8_t>(BlockDirectionState::INBOUND),
              proto::BlockDirectionState_INBOUND);
}

// BlockSectionState C++ and proto have inverted ordinals — document the inversion.
TEST(SnapshotService, BlockSectionStateEnumsAreInverted)
{
    // C++  : CLOSED=0, OPEN=1
    // proto: OPEN=0,   CLOSED=1
    EXPECT_EQ(static_cast<uint8_t>(BlockSectionState::CLOSED), 0u);
    EXPECT_EQ(proto::BlockSectionState_CLOSED, 1);
    EXPECT_EQ(static_cast<uint8_t>(BlockSectionState::OPEN), 1u);
    EXPECT_EQ(proto::BlockSectionState_OPEN, 0);
}

}  // namespace
