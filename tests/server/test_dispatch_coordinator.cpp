// tests/server/test_dispatch_coordinator.cpp
//
// Unit tests for DispatchCoordinator (business logic layer).
// Tests the coordinator directly, without going through DispatchChannel wire parsing.

#include "server/db_writer.hpp"
#include "server/dispatch_coordinator.hpp"
#include "server/dispatch_exchange_manager.hpp"
#include "server/edr_coordinator.hpp"

#include <gtest/gtest.h>

using namespace server;

static constexpr const char* kSrcArea = "GDN_A";
static constexpr const char* kDstArea = "GDN_B";
static constexpr const char* kTrain = "IC-4455";
static constexpr const char* kSession = "coord-test-session";

struct DispatchCoordinatorFixture : ::testing::Test
{
    NullDbWriter db;
    DispatchExchangeManager exchanges;
    EdrCoordinator edr{db, kSession};
    DispatchCoordinator coordinator{exchanges, db, edr, kSession};
};

// ── handle_dispatch_form ──────────────────────────────────────────────────────

TEST_F(DispatchCoordinatorFixture, S2_ReturnsOutcome)
{
    auto result = coordinator.handle_dispatch_form(engine::core::DispatchFormType::S2,
                                                   engine::core::TelegramDirection::SENT, kSrcArea,
                                                   kDstArea, kTrain, std::nullopt, {}, 1000);

    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->exchange_id.empty());
}

TEST_F(DispatchCoordinatorFixture, S2_PersistsRow)
{
    const auto accepted = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S2, engine::core::TelegramDirection::SENT, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 1000);
    ASSERT_TRUE(accepted.has_value());

    ASSERT_EQ(db.written_telegrams.size(), 1u);
    EXPECT_EQ(db.written_telegrams[0].form_type, "S2");
    EXPECT_EQ(db.written_telegrams[0].train_number, kTrain);
    EXPECT_EQ(db.written_telegrams[0].status, "ACCEPTED");
}

TEST_F(DispatchCoordinatorFixture, S24_UpdatesEdrTrackClearTime)
{
    const auto s2 = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S2, engine::core::TelegramDirection::SENT, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 1000);
    ASSERT_TRUE(s2.has_value());
    const auto s24 = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S24, engine::core::TelegramDirection::RECEIVED, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 2000);
    ASSERT_TRUE(s24.has_value());

    ASSERT_EQ(db.edr_updates.size(), 1u);
    EXPECT_EQ(db.edr_updates[0].train_number, kTrain);
    EXPECT_EQ(db.edr_updates[0].station_sid, kDstArea);
}

TEST_F(DispatchCoordinatorFixture, S56_UpdatesEdrTrackClearTime)
{
    const auto s55 = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S55, engine::core::TelegramDirection::SENT, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 1000);
    ASSERT_TRUE(s55.has_value());
    const auto s56 = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S56, engine::core::TelegramDirection::RECEIVED, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 2000);
    ASSERT_TRUE(s56.has_value());

    ASSERT_EQ(db.edr_updates.size(), 1u);
}

TEST_F(DispatchCoordinatorFixture, RejectedTelegram_ReturnsNullopt)
{
    // S24 without preceding S2 — state machine rejects it.
    auto result = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S24, engine::core::TelegramDirection::RECEIVED, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 1000);

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(db.written_telegrams.empty());
}

TEST_F(DispatchCoordinatorFixture, RejectedTelegram_NoEdrUpdate)
{
    const auto rejected = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S24, engine::core::TelegramDirection::RECEIVED, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 1000);
    EXPECT_FALSE(rejected.has_value());

    EXPECT_TRUE(db.edr_updates.empty());
    EXPECT_TRUE(db.edr_departures.empty());
    EXPECT_TRUE(db.edr_arrivals.empty());
}

TEST_F(DispatchCoordinatorFixture, SequentialExchangeIds_Match)
{
    auto r1 = coordinator.handle_dispatch_form(engine::core::DispatchFormType::S2,
                                               engine::core::TelegramDirection::SENT, kSrcArea,
                                               kDstArea, kTrain, std::nullopt, {}, 1000);
    auto r2 = coordinator.handle_dispatch_form(engine::core::DispatchFormType::S24,
                                               engine::core::TelegramDirection::RECEIVED, kSrcArea,
                                               kDstArea, kTrain, std::nullopt, {}, 2000);

    ASSERT_TRUE(r1.has_value());
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r1->exchange_id, r2->exchange_id);
}

TEST_F(DispatchCoordinatorFixture, TrackNumber_Persisted)
{
    const auto accepted = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S2, engine::core::TelegramDirection::SENT, kSrcArea,
        kDstArea, kTrain, std::string{"T1"}, {}, 1000);
    ASSERT_TRUE(accepted.has_value());

    ASSERT_EQ(db.written_telegrams.size(), 1u);
    ASSERT_TRUE(db.written_telegrams[0].track_number.has_value());
    EXPECT_EQ(db.written_telegrams[0].track_number.value(), "T1");
}

TEST_F(DispatchCoordinatorFixture, S25_Sent_EdrDepartureForSrcArea)
{
    const auto s2 = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S2, engine::core::TelegramDirection::SENT, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 1000);
    ASSERT_TRUE(s2.has_value());
    const auto s24 = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S24, engine::core::TelegramDirection::RECEIVED, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 2000);
    ASSERT_TRUE(s24.has_value());
    const auto s25 = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S25, engine::core::TelegramDirection::SENT, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 3000);
    ASSERT_TRUE(s25.has_value());

    ASSERT_EQ(db.edr_departures.size(), 1u);
    EXPECT_EQ(db.edr_departures[0].station_sid, kSrcArea);
}

// ── handle_free_text ─────────────────────────────────────────────────────────

TEST_F(DispatchCoordinatorFixture, FreeText_PersistsRow)
{
    coordinator.handle_free_text(kSrcArea, kDstArea, "Test message", 9999);

    ASSERT_EQ(db.written_telegrams.size(), 1u);
    EXPECT_EQ(db.written_telegrams[0].form_type, "FREE_TEXT");
    EXPECT_EQ(db.written_telegrams[0].body, "Test message");
    EXPECT_EQ(db.written_telegrams[0].from_sid, kSrcArea);
    EXPECT_EQ(db.written_telegrams[0].to_sid, kDstArea);
    EXPECT_EQ(db.written_telegrams[0].status, "ACCEPTED");
}

TEST_F(DispatchCoordinatorFixture, FreeText_NoEdrUpdates)
{
    coordinator.handle_free_text(kSrcArea, kDstArea, "hello", 1000);

    EXPECT_TRUE(db.edr_updates.empty());
    EXPECT_TRUE(db.edr_departures.empty());
    EXPECT_TRUE(db.edr_arrivals.empty());
}

TEST_F(DispatchCoordinatorFixture, FreeText_DoesNotAffectExchangeState)
{
    // Free text should not change the state machine.
    coordinator.handle_free_text(kSrcArea, kDstArea, "hello", 1000);

    // S24 without preceding S2 must still be rejected.
    auto result = coordinator.handle_dispatch_form(
        engine::core::DispatchFormType::S24, engine::core::TelegramDirection::RECEIVED, kSrcArea,
        kDstArea, kTrain, std::nullopt, {}, 2000);

    EXPECT_FALSE(result.has_value());
}
