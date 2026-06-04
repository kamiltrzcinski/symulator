#include <gtest/gtest.h>

#include <engine/core/engine_snapshot.hpp>
#include <engine/core/snapshot_service.hpp>
#include <engine/core/track_model.hpp>
#include <engine/core/types.hpp>

#include <tests/common/param_test_helpers.hpp>

#include "common_generated.h"
#include "snapshot_generated.h"

#include <flatbuffers/flatbuffers.h>

#include <array>
#include <cstdint>
#include <vector>

namespace
{

using namespace engine::core;

constexpr UID kSigX = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1);
constexpr UID kSwY = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 1);
constexpr UID kOtZ = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
constexpr UID kDerWk1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::DERAILER, 1, 1);
constexpr UID kBlSec1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION, 1, 1);
constexpr UID kBlOpen = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION, 1, 2);
constexpr UID kRt1 = make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 1);
constexpr UID kSigY = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 2);
constexpr UID kAlm1 = make_uid(UIDDomain::OPERATIONS, UIDKind::ALARM, 1, 1);

EngineSnapshot make_snapshot()
{
    EngineSnapshot snap;
    snap.session = "SVC_TEST";
    snap.tick = 42;

    Signal sig;
    sig.uid = kSigX;
    sig.pid = "Wx";
    sig.current_aspect = SignalAspect::S2_PROCEED;
    snap.signals[sig.uid] = sig;

    Switch sw;
    sw.uid = kSwY;
    sw.pid = "zy";
    sw.position = SwitchPosition::DIVERGENT;
    sw.locked_by_route_uid = kRt1;
    sw.occupancy = TrackOccupancy::FREE;
    snap.switches[sw.uid] = sw;

    TrackSection ts;
    ts.uid = kOtZ;
    ts.pid = "tz";
    ts.occupancy = TrackOccupancy::OCCUPIED;
    ts.axle_count = 4;
    snap.track_sections[ts.uid] = ts;

    Derailer der;
    der.uid = kDerWk1;
    der.pid = "wk1";
    der.state = DerailerState::UNLOCKED;
    snap.derailers[der.uid] = der;

    BlockSection bs;
    bs.uid = kBlSec1;
    bs.pid = "bl1";
    bs.state = BlockSectionState::CLOSED;
    bs.direction = BlockDirectionState::OUTBOUND;
    snap.block_sections[bs.uid] = bs;

    RouteState rt;
    rt.uid = kRt1;
    rt.from_signal_uid = kSigX;
    rt.to_signal_uid = kSigY;
    rt.section_uids = {kOtZ};
    snap.routes[rt.uid] = rt;

    AlarmState al;
    al.uid = kAlm1;
    al.kind = "SWITCH_POSITION_MISMATCH";
    al.object_uid = kSwY;
    al.message = "Switch disagreement";
    snap.alarms[al.uid] = al;

    return snap;
}

class SnapshotSerializationFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        snap_ = make_snapshot();
        bin_ = SnapshotService::serialize(snap_);
        parsed_ = proto::GetSnapshot(bin_.data());
        ASSERT_NE(parsed_, nullptr);
    }

    EngineSnapshot snap_;
    std::vector<uint8_t> bin_;
    const proto::Snapshot* parsed_ = nullptr;
};

TEST_F(SnapshotSerializationFixture, SerializeProducesBinary)
{
    EXPECT_GT(bin_.size(), 0u);
}

TEST_F(SnapshotSerializationFixture, SerializationIsValidFlatBuffer)
{
    flatbuffers::Verifier verifier(bin_.data(), bin_.size());
    EXPECT_TRUE(proto::VerifySnapshotBuffer(verifier));
}

TEST_F(SnapshotSerializationFixture, SessionAndTickRoundTrip)
{
    ASSERT_NE(parsed_->session_id(), nullptr);
    EXPECT_EQ(parsed_->session_id()->str(), "SVC_TEST");
    EXPECT_EQ(parsed_->seq_cursor(), 42u);
}

using SnapshotAssertion = void (*)(const proto::Snapshot* parsed);

void assert_signal_roundtrip(const proto::Snapshot* parsed)
{
    ASSERT_NE(parsed->signals(), nullptr);
    ASSERT_EQ(parsed->signals()->size(), 1u);
    const auto* sig = parsed->signals()->Get(0);
    ASSERT_NE(sig, nullptr);
    EXPECT_EQ(sig->uid(), kSigX.value);
    EXPECT_EQ(sig->aspect(), proto::Aspect_S2_PROCEED);
}

void assert_switch_roundtrip(const proto::Snapshot* parsed)
{
    ASSERT_NE(parsed->switches(), nullptr);
    ASSERT_EQ(parsed->switches()->size(), 1u);
    const auto* sw = parsed->switches()->Get(0);
    ASSERT_NE(sw, nullptr);
    EXPECT_EQ(sw->uid(), kSwY.value);
    EXPECT_EQ(sw->position(), proto::SwitchPosition_DIVERGENT);
    EXPECT_TRUE(sw->locked_by_route());
    EXPECT_FALSE(sw->occupied());
}

void assert_track_roundtrip(const proto::Snapshot* parsed)
{
    ASSERT_NE(parsed->track_sections(), nullptr);
    ASSERT_EQ(parsed->track_sections()->size(), 1u);
    const auto* ts = parsed->track_sections()->Get(0);
    ASSERT_NE(ts, nullptr);
    EXPECT_EQ(ts->uid(), kOtZ.value);
    EXPECT_TRUE(ts->occupied());
    EXPECT_EQ(ts->axle_count(), 4);
}

void assert_derailer_roundtrip(const proto::Snapshot* parsed)
{
    ASSERT_NE(parsed->derailers(), nullptr);
    ASSERT_EQ(parsed->derailers()->size(), 1u);
    const auto* der = parsed->derailers()->Get(0);
    ASSERT_NE(der, nullptr);
    EXPECT_EQ(der->position(), proto::DerailerPosition_UNLOCKED);
}

void assert_block_roundtrip(const proto::Snapshot* parsed)
{
    ASSERT_NE(parsed->block_sections(), nullptr);
    ASSERT_EQ(parsed->block_sections()->size(), 1u);
    const auto* bs = parsed->block_sections()->Get(0);
    ASSERT_NE(bs, nullptr);
    EXPECT_EQ(bs->state(), proto::BlockSectionState_CLOSED);
    EXPECT_EQ(bs->block_direction(), proto::BlockDirectionState_OUTBOUND);
}

void assert_route_roundtrip(const proto::Snapshot* parsed)
{
    ASSERT_NE(parsed->active_routes(), nullptr);
    ASSERT_EQ(parsed->active_routes()->size(), 1u);
    const auto* rt = parsed->active_routes()->Get(0);
    ASSERT_NE(rt, nullptr);
    EXPECT_EQ(rt->uid(), kRt1.value);
    EXPECT_EQ(rt->from_signal_uid(), kSigX.value);
    EXPECT_EQ(rt->to_signal_uid(), kSigY.value);
    ASSERT_NE(rt->section_uids(), nullptr);
    ASSERT_EQ(rt->section_uids()->size(), 1u);
    EXPECT_EQ(rt->section_uids()->Get(0), kOtZ.value);
}

void assert_alarm_roundtrip(const proto::Snapshot* parsed)
{
    ASSERT_NE(parsed->active_alarms(), nullptr);
    ASSERT_EQ(parsed->active_alarms()->size(), 1u);
    const auto* al = parsed->active_alarms()->Get(0);
    ASSERT_NE(al, nullptr);
    EXPECT_EQ(al->uid(), kAlm1.value);
    EXPECT_EQ(al->alarm_type(), 1);
    ASSERT_NE(al->message(), nullptr);
    EXPECT_EQ(al->message()->str(), "Switch disagreement");
}

struct SnapshotRoundtripCase
{
    const char* name;
    SnapshotAssertion assert_fn;
};

class SnapshotRoundtripTest : public SnapshotSerializationFixture,
                              public ::testing::WithParamInterface<SnapshotRoundtripCase>
{
};

TEST_P(SnapshotRoundtripTest, EntityRoundTrip)
{
    GetParam().assert_fn(parsed_);
}

INSTANTIATE_TEST_SUITE_P(
    EntityRoundTrips, SnapshotRoundtripTest,
    ::testing::Values(SnapshotRoundtripCase{"Signal", &assert_signal_roundtrip},
                      SnapshotRoundtripCase{"Switch", &assert_switch_roundtrip},
                      SnapshotRoundtripCase{"TrackSection", &assert_track_roundtrip},
                      SnapshotRoundtripCase{"Derailer", &assert_derailer_roundtrip},
                      SnapshotRoundtripCase{"BlockSection", &assert_block_roundtrip},
                      SnapshotRoundtripCase{"Route", &assert_route_roundtrip},
                      SnapshotRoundtripCase{"Alarm", &assert_alarm_roundtrip}),
    tests::common::param_name<SnapshotRoundtripCase>);

TEST(SnapshotService, BlockSectionStateEnumMappingIsExplicit)
{
    EngineSnapshot snap;
    snap.session = "MAP";

    BlockSection open_bs;
    open_bs.uid = kBlOpen;
    open_bs.pid = "blo";
    open_bs.state = BlockSectionState::OPEN;
    open_bs.direction = BlockDirectionState::NEUTRAL;
    snap.block_sections[open_bs.uid] = open_bs;

    const auto bin = SnapshotService::serialize(snap);
    const auto* parsed = proto::GetSnapshot(bin.data());
    ASSERT_NE(parsed, nullptr);
    ASSERT_NE(parsed->block_sections(), nullptr);
    ASSERT_EQ(parsed->block_sections()->size(), 1u);
    const auto* bs = parsed->block_sections()->Get(0);
    ASSERT_NE(bs, nullptr);
    EXPECT_EQ(bs->state(), proto::BlockSectionState_OPEN);
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

struct ChunkCase
{
    const char* name;
    std::size_t binary_size;
    std::size_t chunk_size;
    std::array<std::size_t, 4> expected_sizes;
    std::size_t expected_chunk_count;
};

class SnapshotChunkingTest : public ::testing::TestWithParam<ChunkCase>
{
};

TEST_P(SnapshotChunkingTest, SplitsAtExpectedBoundaries)
{
    const auto param = GetParam();
    std::vector<uint8_t> binary(param.binary_size, 0xAB);
    const auto chunks = SnapshotService::chunk(binary, param.chunk_size);

    ASSERT_EQ(chunks.size(), param.expected_chunk_count);
    for (std::size_t i = 0; i < chunks.size(); ++i)
    {
        EXPECT_EQ(chunks[i].size(), param.expected_sizes[i]);
    }
}

INSTANTIATE_TEST_SUITE_P(
    FixedSizeChunkCases, SnapshotChunkingTest,
    ::testing::Values(ChunkCase{"SingleChunk", 100, 1024, {100, 0, 0, 0}, 1},
                      ChunkCase{"ExactBoundary", 300, 100, {100, 100, 100, 0}, 3},
                      ChunkCase{"TrailingRemainder", 250, 100, {100, 100, 50, 0}, 3}),
    tests::common::param_name<ChunkCase>);

struct ChunkReassembleCase
{
    const char* name;
    std::size_t binary_size;
    std::size_t chunk_size;
};

class SnapshotChunkReassembleTest : public ::testing::TestWithParam<ChunkReassembleCase>
{
};

TEST_P(SnapshotChunkReassembleTest, PreservesData)
{
    const auto param = GetParam();
    std::vector<uint8_t> binary(param.binary_size);
    for (std::size_t i = 0; i < binary.size(); ++i)
    {
        binary[i] = static_cast<uint8_t>(i % 256);
    }

    const auto chunks = SnapshotService::chunk(binary, param.chunk_size);

    std::vector<uint8_t> reassembled;
    for (const auto& chunk : chunks)
    {
        reassembled.insert(reassembled.end(), chunk.begin(), chunk.end());
    }

    EXPECT_EQ(reassembled, binary);
}

INSTANTIATE_TEST_SUITE_P(ReassembleCases, SnapshotChunkReassembleTest,
                         ::testing::Values(ChunkReassembleCase{"ReassembleMedium", 200, 80},
                                           ChunkReassembleCase{"ReassembleSmall", 31, 8},
                                           ChunkReassembleCase{"ReassembleLarge", 2049, 257}),
                         tests::common::param_name<ChunkReassembleCase>);

TEST(SnapshotService, ChunkRealSnapshotFitsInDefaultChunks)
{
    const auto snap = make_snapshot();
    const auto bin = SnapshotService::serialize(snap);
    const auto chunks = SnapshotService::chunk(bin);

    std::vector<uint8_t> reassembled;
    for (const auto& chunk : chunks)
    {
        EXPECT_LE(chunk.size(), SnapshotService::DEFAULT_CHUNK_SIZE);
        reassembled.insert(reassembled.end(), chunk.begin(), chunk.end());
    }

    flatbuffers::Verifier verifier(reassembled.data(), reassembled.size());
    EXPECT_TRUE(proto::VerifySnapshotBuffer(verifier));
}

struct EnumAlignmentCase
{
    const char* name;
    std::uint8_t cpp_value;
    std::int32_t proto_value;
};

class SnapshotEnumAlignmentTest : public ::testing::TestWithParam<EnumAlignmentCase>
{
};

TEST_P(SnapshotEnumAlignmentTest, ValuesAreAligned)
{
    const auto param = GetParam();
    EXPECT_EQ(param.cpp_value, param.proto_value);
}

INSTANTIATE_TEST_SUITE_P(
    EnumAlignment, SnapshotEnumAlignmentTest,
    ::testing::Values(
        EnumAlignmentCase{"SignalS1Stop", static_cast<std::uint8_t>(SignalAspect::S1_STOP),
                          proto::Aspect_S1_STOP},
        EnumAlignmentCase{"SignalS2Proceed", static_cast<std::uint8_t>(SignalAspect::S2_PROCEED),
                          proto::Aspect_S2_PROCEED},
        EnumAlignmentCase{"SignalMs1Stop", static_cast<std::uint8_t>(SignalAspect::MS1_STOP),
                          proto::Aspect_MS1_STOP},
        EnumAlignmentCase{"SignalMs2ShuntingAllowed",
                          static_cast<std::uint8_t>(SignalAspect::MS2_SHUNTING_ALLOWED),
                          proto::Aspect_MS2_SHUNTING_ALLOWED},
        EnumAlignmentCase{"SwitchStraight", static_cast<std::uint8_t>(SwitchPosition::STRAIGHT),
                          proto::SwitchPosition_STRAIGHT},
        EnumAlignmentCase{"SwitchDivergent", static_cast<std::uint8_t>(SwitchPosition::DIVERGENT),
                          proto::SwitchPosition_DIVERGENT},
        EnumAlignmentCase{"SwitchMoving", static_cast<std::uint8_t>(SwitchPosition::MOVING),
                          proto::SwitchPosition_MOVING},
        EnumAlignmentCase{"BlockDirectionNeutral",
                          static_cast<std::uint8_t>(BlockDirectionState::NEUTRAL),
                          proto::BlockDirectionState_NEUTRAL},
        EnumAlignmentCase{"BlockDirectionOutbound",
                          static_cast<std::uint8_t>(BlockDirectionState::OUTBOUND),
                          proto::BlockDirectionState_OUTBOUND},
        EnumAlignmentCase{"BlockDirectionInbound",
                          static_cast<std::uint8_t>(BlockDirectionState::INBOUND),
                          proto::BlockDirectionState_INBOUND}),
    tests::common::param_name<EnumAlignmentCase>);

TEST(SnapshotService, BlockSectionStateEnumsAreInverted)
{
    EXPECT_EQ(static_cast<std::uint8_t>(BlockSectionState::CLOSED), 0u);
    EXPECT_EQ(proto::BlockSectionState_CLOSED, 1);
    EXPECT_EQ(static_cast<std::uint8_t>(BlockSectionState::OPEN), 1u);
    EXPECT_EQ(proto::BlockSectionState_OPEN, 0);
}

}  // namespace
