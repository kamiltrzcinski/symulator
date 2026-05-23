// tests/server/test_bilateral_channel.cpp
//
// Unit tests for BilateralChannel.
// Uses NullDbWriter to assert DB write side-effects.
// TransportGateway is constructed but never started — broadcast_to_pair posts
// to an idle io_context (safe, no UB).  We verify only DB side-effects here.

#include "server/bilateral_channel.hpp"
#include "server/db_writer.hpp"
#include "server/dispatch_exchange_manager.hpp"
#include "server/ownership_guard.hpp"
#include "server/transport_gateway.hpp"

#include "bilateral_generated.h"
#include "common_generated.h"

#include "engine/core/engine_snapshot.hpp"
#include "engine/core/priority_command_queue.hpp"

#include <flatbuffers/flatbuffers.h>
#include <gtest/gtest.h>

using namespace server;

static constexpr const char* kSrcArea = "GGO_A";
static constexpr const char* kDstArea = "GOP_B";
static constexpr const char* kTrain = "TLK-1234";
static constexpr const char* kSession = "test-session-0001";
static constexpr const char* kClient = "player-1";

// ── Payload builders ──────────────────────────────────────────────────────────

static std::vector<uint8_t> make_dispatch_form(const char* src, const char* dst,
                                               proto::DispatchFormType form,
                                               proto::TelegramDirection dir, const char* train)
{
    flatbuffers::FlatBufferBuilder fbb(256);
    auto src_off = fbb.CreateString(src);
    auto dst_off = fbb.CreateString(dst);
    auto train_off = fbb.CreateString(train);
    auto dfp_off = proto::CreateDispatchFormPayload(fbb, form, train_off);
    auto root = proto::CreateBilateralMessage(
        fbb, src_off, dst_off, dir, proto::BilateralKind_DISPATCH_FORM,
        proto::BilateralBody_DispatchFormPayload, dfp_off.Union());
    fbb.Finish(root);
    return {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()};
}

static std::vector<uint8_t> make_free_text(const char* src, const char* dst, const char* body_str)
{
    flatbuffers::FlatBufferBuilder fbb(256);
    auto src_off = fbb.CreateString(src);
    auto dst_off = fbb.CreateString(dst);
    auto body_off = fbb.CreateString(body_str);
    auto ft_off = proto::CreateFreeTextPayload(fbb, body_off);
    auto root = proto::CreateBilateralMessage(fbb, src_off, dst_off, proto::TelegramDirection_SENT,
                                              proto::BilateralKind_FREE_TEXT,
                                              proto::BilateralBody_FreeTextPayload, ft_off.Union());
    fbb.Finish(root);
    return {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()};
}

// ── Fixture ───────────────────────────────────────────────────────────────────

struct BilateralChannelFixture : ::testing::Test
{
    engine::core::PriorityCommandQueue<engine::core::EnvelopedCommand> cmd_queue;
    OwnershipGuard ownership;
    engine::core::AtomicSnapshot snapshot;

    TransportGateway gateway{cmd_queue, ownership, snapshot};
    DispatchExchangeManager exchanges;
    NullDbWriter db;
    BilateralChannel channel{exchanges, db, gateway, kSession};

    void send(const std::vector<uint8_t>& payload, const char* area = kSrcArea)
    {
        channel.on_inbound(payload, kClient, area);
    }
};

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST_F(BilateralChannelFixture, S2_Accepted_WritesOneRow)
{
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S2,
                            proto::TelegramDirection_SENT, kTrain));

    ASSERT_EQ(db.written_telegrams.size(), 1u);
    EXPECT_EQ(db.written_telegrams[0].form_type, "S2");
    EXPECT_EQ(db.written_telegrams[0].train_number, kTrain);
    EXPECT_EQ(db.written_telegrams[0].from_sid, kSrcArea);
    EXPECT_EQ(db.written_telegrams[0].to_sid, kDstArea);
    EXPECT_EQ(db.written_telegrams[0].status, "ACCEPTED");
    EXPECT_FALSE(db.written_telegrams[0].exchange_id.empty());
    EXPECT_TRUE(db.edr_updates.empty());
}

TEST_F(BilateralChannelFixture, S24_Accepted_WritesRowAndEdrUpdate)
{
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S2,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S24,
                            proto::TelegramDirection_RECEIVED, kTrain));

    ASSERT_EQ(db.written_telegrams.size(), 2u);
    EXPECT_EQ(db.written_telegrams[1].form_type, "S24");
    ASSERT_EQ(db.edr_updates.size(), 1u);
    EXPECT_EQ(db.edr_updates[0].train_number, kTrain);
    EXPECT_EQ(db.edr_updates[0].station_sid, kDstArea);
}

TEST_F(BilateralChannelFixture, HappyPath_S2_S24_S25_S26_WritesAll)
{
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S2,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S24,
                            proto::TelegramDirection_RECEIVED, kTrain));
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S25,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S26,
                            proto::TelegramDirection_RECEIVED, kTrain));

    EXPECT_EQ(db.written_telegrams.size(), 4u);
    EXPECT_EQ(db.edr_updates.size(), 1u);
}

TEST_F(BilateralChannelFixture, S55_S56_Accepted_WritesEdrUpdate)
{
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S55,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S56,
                            proto::TelegramDirection_RECEIVED, kTrain));

    ASSERT_EQ(db.edr_updates.size(), 1u);
    EXPECT_EQ(db.written_telegrams[1].form_type, "S56");
}

TEST_F(BilateralChannelFixture, RejectedTelegram_NoDbWrite)
{
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S24,
                            proto::TelegramDirection_RECEIVED, kTrain));

    EXPECT_TRUE(db.written_telegrams.empty());
    EXPECT_TRUE(db.edr_updates.empty());
}

TEST_F(BilateralChannelFixture, FreeText_WritesOneRow_NoEdrUpdate)
{
    send(make_free_text(kSrcArea, kDstArea, "Uwaga — opoznienie 15 min"));

    ASSERT_EQ(db.written_telegrams.size(), 1u);
    EXPECT_EQ(db.written_telegrams[0].form_type, "FREE_TEXT");
    EXPECT_EQ(db.written_telegrams[0].body, "Uwaga — opoznienie 15 min");
    EXPECT_TRUE(db.edr_updates.empty());
}

TEST_F(BilateralChannelFixture, SpoofedSrcArea_Dropped)
{
    auto payload = make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S2,
                                      proto::TelegramDirection_SENT, kTrain);

    channel.on_inbound(payload, kClient, kDstArea);

    EXPECT_TRUE(db.written_telegrams.empty());
}

TEST_F(BilateralChannelFixture, MalformedPayload_Dropped)
{
    send({0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01});
    EXPECT_TRUE(db.written_telegrams.empty());
}

TEST_F(BilateralChannelFixture, EmptyPayload_Dropped)
{
    send({});
    EXPECT_TRUE(db.written_telegrams.empty());
}

TEST_F(BilateralChannelFixture, AllTelegramsSameExchange_SameExchangeId)
{
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S2,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcArea, kDstArea, proto::DispatchFormType_S24,
                            proto::TelegramDirection_RECEIVED, kTrain));

    ASSERT_GE(db.written_telegrams.size(), 2u);
    EXPECT_EQ(db.written_telegrams[0].exchange_id, db.written_telegrams[1].exchange_id);
    EXPECT_FALSE(db.written_telegrams[0].exchange_id.empty());
}
