// tests/server/test_dispatch_channel.cpp
//
// Unit tests for DispatchChannel (wire-protocol layer).
// Uses NullDbWriter to assert DB write side-effects.
// TransportGateway is constructed but never started — broadcast_to_pair posts
// to an idle io_context (safe, no UB).  We verify only DB side-effects here.

#include "server/dispatch_channel.hpp"
#include "server/dispatch_coordinator.hpp"
#include "server/db_writer.hpp"
#include "server/dispatch_exchange_manager.hpp"
#include "server/edr_coordinator.hpp"
#include "server/ownership_guard.hpp"
#include "server/transport_gateway.hpp"

#include "dispatch_channel_generated.h"
#include "common_generated.h"

#include "engine/core/engine_snapshot.hpp"
#include "engine/core/priority_command_queue.hpp"

#include <flatbuffers/flatbuffers.h>
#include <gtest/gtest.h>

#include <string>

using namespace server;

// Area UIDs for test: dispatch_channel.cpp converts them to std::to_string for internal routing.
// The sender_area_id parameter must match std::to_string(src_area_uid).
static constexpr uint64_t kSrcAreaUid = 2305843009213693953ULL;  // arbitrary valid uint64
static constexpr uint64_t kDstAreaUid = 2305843009213693954ULL;

// Numeric-string equivalents used for DB assertion comparisons.
static const std::string kSrcArea = std::to_string(kSrcAreaUid);
static const std::string kDstArea = std::to_string(kDstAreaUid);

static constexpr const char* kTrain = "TLK-1234";
static constexpr const char* kSession = "test-session-0001";
static constexpr const char* kClient = "player-1";

// ── Payload builders ──────────────────────────────────────────────────────────

static std::vector<uint8_t> make_dispatch_form(uint64_t src, uint64_t dst,
                                               proto::DispatchFormType form,
                                               proto::TelegramDirection dir, const char* train)
{
    flatbuffers::FlatBufferBuilder fbb(256);
    auto train_off = fbb.CreateString(train);
    auto dfp_off = proto::CreateDispatchFormPayload(fbb, form, train_off);
    auto root = proto::CreateDispatchChannelMessage(
        fbb, src, dst, dir, proto::DispatchChannelMessageKind_DISPATCH_FORM,
        proto::DispatchChannelMessageBody_DispatchFormPayload, dfp_off.Union());
    fbb.Finish(root);
    return {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()};
}

static std::vector<uint8_t> make_free_text(uint64_t src, uint64_t dst, const char* body_str)
{
    flatbuffers::FlatBufferBuilder fbb(256);
    auto body_off = fbb.CreateString(body_str);
    auto ft_off = proto::CreateFreeTextPayload(fbb, body_off);
    auto root = proto::CreateDispatchChannelMessage(
        fbb, src, dst, proto::TelegramDirection_SENT, proto::DispatchChannelMessageKind_FREE_TEXT,
        proto::DispatchChannelMessageBody_FreeTextPayload, ft_off.Union());
    fbb.Finish(root);
    return {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()};
}

// ── Fixture ───────────────────────────────────────────────────────────────────

struct DispatchChannelFixture : ::testing::Test
{
    engine::core::PriorityCommandQueue<engine::core::EnvelopedCommand> cmd_queue;
    OwnershipGuard ownership;
    engine::core::AtomicSnapshot snapshot;

    TransportGateway gateway{cmd_queue, ownership, snapshot};
    DispatchExchangeManager exchanges;
    NullDbWriter db;
    EdrCoordinator edr{db, kSession};
    DispatchCoordinator coordinator{exchanges, db, edr, kSession};
    DispatchChannel channel{coordinator, gateway};

    void send(const std::vector<uint8_t>& payload, const std::string& area = kSrcArea)
    {
        channel.on_inbound(payload, kClient, area);
    }
};

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST_F(DispatchChannelFixture, S2_Accepted_WritesOneRow)
{
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S2,
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

TEST_F(DispatchChannelFixture, S24_Accepted_WritesRowAndEdrUpdate)
{
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S2,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S24,
                            proto::TelegramDirection_RECEIVED, kTrain));

    ASSERT_EQ(db.written_telegrams.size(), 2u);
    EXPECT_EQ(db.written_telegrams[1].form_type, "S24");
    ASSERT_EQ(db.edr_updates.size(), 1u);
    EXPECT_EQ(db.edr_updates[0].train_number, kTrain);
    EXPECT_EQ(db.edr_updates[0].station_sid, kDstArea);
}

TEST_F(DispatchChannelFixture, HappyPath_S2_S24_S25_S26_WritesAll)
{
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S2,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S24,
                            proto::TelegramDirection_RECEIVED, kTrain));
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S25,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S26,
                            proto::TelegramDirection_RECEIVED, kTrain));

    EXPECT_EQ(db.written_telegrams.size(), 4u);
    EXPECT_EQ(db.edr_updates.size(), 1u);
    EXPECT_EQ(db.edr_departures.size(), 1u);
    EXPECT_EQ(db.edr_arrivals.size(), 1u);
}

TEST_F(DispatchChannelFixture, S55_S56_Accepted_WritesEdrUpdate)
{
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S55,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S56,
                            proto::TelegramDirection_RECEIVED, kTrain));

    ASSERT_EQ(db.edr_updates.size(), 1u);
    EXPECT_EQ(db.written_telegrams[1].form_type, "S56");
}

TEST_F(DispatchChannelFixture, RejectedTelegram_NoDbWrite)
{
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S24,
                            proto::TelegramDirection_RECEIVED, kTrain));

    EXPECT_TRUE(db.written_telegrams.empty());
    EXPECT_TRUE(db.edr_updates.empty());
}

TEST_F(DispatchChannelFixture, FreeText_WritesOneRow_NoEdrUpdate)
{
    send(make_free_text(kSrcAreaUid, kDstAreaUid, "Uwaga — opoznienie 15 min"));

    ASSERT_EQ(db.written_telegrams.size(), 1u);
    EXPECT_EQ(db.written_telegrams[0].form_type, "FREE_TEXT");
    EXPECT_EQ(db.written_telegrams[0].body, "Uwaga — opoznienie 15 min");
    EXPECT_TRUE(db.edr_updates.empty());
}

TEST_F(DispatchChannelFixture, SpoofedSrcArea_Dropped)
{
    auto payload = make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S2,
                                      proto::TelegramDirection_SENT, kTrain);
    // Authenticate as kDstArea but message claims kSrcArea — mismatch, dropped.
    channel.on_inbound(payload, kClient, kDstArea);

    EXPECT_TRUE(db.written_telegrams.empty());
}

TEST_F(DispatchChannelFixture, MalformedPayload_Dropped)
{
    send({0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01});
    EXPECT_TRUE(db.written_telegrams.empty());
}

TEST_F(DispatchChannelFixture, EmptyPayload_Dropped)
{
    send({});
    EXPECT_TRUE(db.written_telegrams.empty());
}

TEST_F(DispatchChannelFixture, AllTelegramsSameExchange_SameExchangeId)
{
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S2,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S24,
                            proto::TelegramDirection_RECEIVED, kTrain));

    ASSERT_GE(db.written_telegrams.size(), 2u);
    EXPECT_EQ(db.written_telegrams[0].exchange_id, db.written_telegrams[1].exchange_id);
    EXPECT_FALSE(db.written_telegrams[0].exchange_id.empty());
}

TEST_F(DispatchChannelFixture, S25_Sent_SetsEdrDepartureForSrcArea)
{
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S2,
                            proto::TelegramDirection_SENT, kTrain));
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S24,
                            proto::TelegramDirection_RECEIVED, kTrain));
    send(make_dispatch_form(kSrcAreaUid, kDstAreaUid, proto::DispatchFormType_S25,
                            proto::TelegramDirection_SENT, kTrain));

    ASSERT_EQ(db.edr_departures.size(), 1u);
    EXPECT_EQ(db.edr_departures[0].station_sid, kSrcArea);
}
