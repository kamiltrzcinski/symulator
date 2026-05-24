// tests/server/test_dispatch_exchange_manager.cpp
//
// Unit tests for DispatchExchangeManager state machine.
// Each test drives one path through the S-form protocol and asserts the
// resulting ExchangeStatus and TelegramResult.

#include "server/dispatch_exchange_manager.hpp"

#include <gtest/gtest.h>
#include <tuple>

using namespace server;
using engine::core::DispatchFormType;
using engine::core::ExchangeStatus;
using engine::core::TelegramDirection;
using engine::core::TelegramStatus;

// ── Helpers ───────────────────────────────────────────────────────────────────

static constexpr const char* kSrcArea = "GGO_nastawnia_A";
static constexpr const char* kDstArea = "GOP_nastawnia_B";
static constexpr const char* kTrain = "TLK-43012";

// ── Initial state ─────────────────────────────────────────────────────────────

TEST(DispatchExchangeManager, InitialStatusIsIdle)
{
    DispatchExchangeManager m;
    EXPECT_EQ(m.status(kSrcArea, kDstArea), ExchangeStatus::IDLE);
}

// ── Happy path: S2 → S24 → S25 → S26 → close() ───────────────────────────────

TEST(DispatchExchangeManager, HappyPath_S2_to_Close)
{
    DispatchExchangeManager m;

    // S2 sent — dispatch request
    auto r1 = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2, TelegramDirection::SENT,
                                kTrain);
    EXPECT_EQ(r1.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r1.new_status, ExchangeStatus::S2_SENT);
    EXPECT_FALSE(r1.exchange_id.empty());

    // S24 received — line clear
    auto r2 = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S24,
                                TelegramDirection::RECEIVED, kTrain);
    EXPECT_EQ(r2.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r2.new_status, ExchangeStatus::S24_RECEIVED);
    EXPECT_EQ(r2.exchange_id, r1.exchange_id);

    // S25 sent — departure notification
    auto r3 = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S25, TelegramDirection::SENT,
                                kTrain);
    EXPECT_EQ(r3.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r3.new_status, ExchangeStatus::S25_SENT);

    // S26 received — arrival confirmation
    auto r4 = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S26,
                                TelegramDirection::RECEIVED, kTrain);
    EXPECT_EQ(r4.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r4.new_status, ExchangeStatus::S26_RECEIVED);

    // close() advances to CLOSED
    m.close(kSrcArea, kDstArea);
    EXPECT_EQ(m.status(kSrcArea, kDstArea), ExchangeStatus::CLOSED);
}

// ── Dangerous-goods path: S55 / S56 ──────────────────────────────────────────

TEST(DispatchExchangeManager, DangerousGoodsPath_S55_S56)
{
    DispatchExchangeManager m;

    auto r1 = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S55, TelegramDirection::SENT,
                                kTrain);
    EXPECT_EQ(r1.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r1.new_status, ExchangeStatus::S2_SENT);

    auto r2 = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S56,
                                TelegramDirection::RECEIVED, kTrain);
    EXPECT_EQ(r2.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r2.new_status, ExchangeStatus::S24_RECEIVED);
}

// ── Cancellation path: S2 → S35 ──────────────────────────────────────────────

TEST(DispatchExchangeManager, CancellationPath_S35)
{
    DispatchExchangeManager m;

    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2,
                                    TelegramDirection::SENT, kTrain);
    ASSERT_EQ(m.status(kSrcArea, kDstArea), ExchangeStatus::S2_SENT);

    auto r = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S35, TelegramDirection::SENT,
                               kTrain);
    EXPECT_EQ(r.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r.new_status, ExchangeStatus::CANCELLED);
}

// ── New exchange after CLOSED ─────────────────────────────────────────────────

TEST(DispatchExchangeManager, NewExchangeAfterClosed)
{
    DispatchExchangeManager m;

    // Run through to CLOSED
    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2,
                                    TelegramDirection::SENT, kTrain);
    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S24,
                                    TelegramDirection::RECEIVED, kTrain);
    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S25,
                                    TelegramDirection::SENT, kTrain);
    auto r26 = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S26,
                                 TelegramDirection::RECEIVED, kTrain);
    m.close(kSrcArea, kDstArea);

    // Start a fresh exchange
    auto r2 = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2, TelegramDirection::SENT,
                                "IC-1234");
    EXPECT_EQ(r2.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r2.new_status, ExchangeStatus::S2_SENT);
    // New exchange gets a new exchange_id
    EXPECT_NE(r2.exchange_id, r26.exchange_id);
}

// ── New exchange after CANCELLED ──────────────────────────────────────────────

TEST(DispatchExchangeManager, NewExchangeAfterCancelled)
{
    DispatchExchangeManager m;

    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2,
                                    TelegramDirection::SENT, kTrain);
    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S35,
                                    TelegramDirection::SENT, kTrain);
    ASSERT_EQ(m.status(kSrcArea, kDstArea), ExchangeStatus::CANCELLED);

    auto r = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2, TelegramDirection::SENT,
                               "IC-1234");
    EXPECT_EQ(r.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r.new_status, ExchangeStatus::S2_SENT);
}

// ── Duplicate guards ──────────────────────────────────────────────────────────

TEST(DispatchExchangeManager, DuplicateS2IsRejected)
{
    DispatchExchangeManager m;

    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2,
                                    TelegramDirection::SENT, kTrain);

    auto r = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2, TelegramDirection::SENT,
                               kTrain);
    EXPECT_EQ(r.result, TelegramResult::REJECTED_DUPLICATE);
    EXPECT_EQ(r.new_status, ExchangeStatus::S2_SENT);  // unchanged
    EXPECT_EQ(r.telegram_status, TelegramStatus::REJECTED);
}

TEST(DispatchExchangeManager, DuplicateS24IsRejected)
{
    DispatchExchangeManager m;

    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2,
                                    TelegramDirection::SENT, kTrain);
    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S24,
                                    TelegramDirection::RECEIVED, kTrain);

    auto r = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S24,
                               TelegramDirection::RECEIVED, kTrain);
    EXPECT_EQ(r.result, TelegramResult::REJECTED_DUPLICATE);
    EXPECT_EQ(r.new_status, ExchangeStatus::S24_RECEIVED);  // unchanged
}

// ── Out-of-order rejection ────────────────────────────────────────────────────

TEST(DispatchExchangeManager, S24BeforeS2IsRejected)
{
    DispatchExchangeManager m;

    auto r = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S24,
                               TelegramDirection::RECEIVED, kTrain);
    EXPECT_EQ(r.result, TelegramResult::REJECTED_WRONG_STATE);
    EXPECT_EQ(r.new_status, ExchangeStatus::IDLE);
}

TEST(DispatchExchangeManager, S25BeforeS24IsRejected)
{
    DispatchExchangeManager m;

    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2,
                                    TelegramDirection::SENT, kTrain);

    auto r = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S25, TelegramDirection::SENT,
                               kTrain);
    EXPECT_EQ(r.result, TelegramResult::REJECTED_WRONG_STATE);
    EXPECT_EQ(r.new_status, ExchangeStatus::S2_SENT);  // unchanged
}

TEST(DispatchExchangeManager, S35FromIdleIsRejected)
{
    DispatchExchangeManager m;

    auto r = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S35, TelegramDirection::SENT,
                               kTrain);
    EXPECT_EQ(r.result, TelegramResult::REJECTED_WRONG_STATE);
}

// ── Independent direction pairs ───────────────────────────────────────────────

TEST(DispatchExchangeManager, DirectionsAreIndependent)
{
    DispatchExchangeManager m;

    // A → B: start exchange
    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2,
                                    TelegramDirection::SENT, kTrain);

    // B → A: completely separate, still IDLE
    EXPECT_EQ(m.status(kDstArea, kSrcArea), ExchangeStatus::IDLE);
}

// ── close() is a no-op except from S26_RECEIVED ───────────────────────────────

TEST(DispatchExchangeManager, CloseNoOpWhenIdle)
{
    DispatchExchangeManager m;
    m.close(kSrcArea, kDstArea);
    EXPECT_EQ(m.status(kSrcArea, kDstArea), ExchangeStatus::IDLE);
}

TEST(DispatchExchangeManager, CloseNoOpWhenS2Sent)
{
    DispatchExchangeManager m;
    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2,
                                    TelegramDirection::SENT, kTrain);
    m.close(kSrcArea, kDstArea);
    EXPECT_EQ(m.status(kSrcArea, kDstArea), ExchangeStatus::S2_SENT);
}

// ── S51 supplementary notifications ──────────────────────────────────────────

TEST(DispatchExchangeManager, S51AcceptedWhenExchangeActive)
{
    DispatchExchangeManager m;
    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2,
                                    TelegramDirection::SENT, kTrain);

    auto r = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S51, TelegramDirection::SENT,
                               kTrain);
    EXPECT_EQ(r.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r.new_status, ExchangeStatus::S2_SENT);  // status unchanged
}

TEST(DispatchExchangeManager, S51RejectedWhenIdle)
{
    DispatchExchangeManager m;

    auto r = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S51, TelegramDirection::SENT,
                               kTrain);
    EXPECT_EQ(r.result, TelegramResult::REJECTED_WRONG_STATE);
}

// ── Exchange IDs are unique across exchanges ──────────────────────────────────

TEST(DispatchExchangeManager, ExchangeIdsAreUnique)
{
    DispatchExchangeManager m;

    auto r1 = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2, TelegramDirection::SENT,
                                kTrain);
    std::ignore = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S35,
                                    TelegramDirection::SENT, kTrain);

    auto r2 = m.submit_telegram(kSrcArea, kDstArea, DispatchFormType::S2, TelegramDirection::SENT,
                                kTrain);

    EXPECT_NE(r1.exchange_id, r2.exchange_id);
    EXPECT_FALSE(r2.exchange_id.empty());
}
