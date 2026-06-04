#include "server/dispatch_exchange_manager.hpp"

#include <gtest/gtest.h>

#include <tests/common/param_test_helpers.hpp>

#include <tuple>

namespace
{

using namespace server;
using engine::core::DispatchFormType;
using engine::core::ExchangeStatus;
using engine::core::TelegramDirection;
using engine::core::TelegramStatus;

static constexpr const char* kSrcArea = "GGO_nastawnia_A";
static constexpr const char* kDstArea = "GOP_nastawnia_B";
static constexpr const char* kTrain = "TLK-43012";
static constexpr const char* kNextTrain = "IC-1234";

enum class SetupScenario
{
    Idle,
    S2Sent,
    S24Received,
};

class DispatchExchangeManagerFixture : public ::testing::Test
{
protected:
    DispatchExchangeManager manager;

    TelegramOutcome submit(DispatchFormType form, TelegramDirection direction,
                           const char* train = kTrain)
    {
        return manager.submit_telegram(kSrcArea, kDstArea, form, direction, train);
    }

    void start_s2()
    {
        const auto r = submit(DispatchFormType::S2, TelegramDirection::SENT);
        ASSERT_EQ(r.result, TelegramResult::ACCEPTED);
        ASSERT_EQ(r.new_status, ExchangeStatus::S2_SENT);
    }

    void progress_to_s24()
    {
        start_s2();
        const auto r = submit(DispatchFormType::S24, TelegramDirection::RECEIVED);
        ASSERT_EQ(r.result, TelegramResult::ACCEPTED);
        ASSERT_EQ(r.new_status, ExchangeStatus::S24_RECEIVED);
    }

    TelegramOutcome progress_to_s26()
    {
        progress_to_s24();
        {
            const auto r = submit(DispatchFormType::S25, TelegramDirection::SENT);
            EXPECT_EQ(r.result, TelegramResult::ACCEPTED);
            EXPECT_EQ(r.new_status, ExchangeStatus::S25_SENT);
        }

        const auto r = submit(DispatchFormType::S26, TelegramDirection::RECEIVED);
        EXPECT_EQ(r.result, TelegramResult::ACCEPTED);
        EXPECT_EQ(r.new_status, ExchangeStatus::S26_RECEIVED);
        return r;
    }

    void prepare_state(SetupScenario scenario)
    {
        switch (scenario)
        {
            case SetupScenario::Idle:
                break;
            case SetupScenario::S2Sent:
                start_s2();
                break;
            case SetupScenario::S24Received:
                progress_to_s24();
                break;
        }
    }
};

TEST_F(DispatchExchangeManagerFixture, InitialStatusIsIdle)
{
    EXPECT_EQ(manager.status(kSrcArea, kDstArea), ExchangeStatus::IDLE);
}

TEST_F(DispatchExchangeManagerFixture, HappyPathS2ToClose)
{
    const auto r26 = progress_to_s26();
    EXPECT_FALSE(r26.exchange_id.empty());

    manager.close(kSrcArea, kDstArea);
    EXPECT_EQ(manager.status(kSrcArea, kDstArea), ExchangeStatus::CLOSED);
}

TEST_F(DispatchExchangeManagerFixture, DangerousGoodsPathS55S56)
{
    const auto r1 = submit(DispatchFormType::S55, TelegramDirection::SENT);
    EXPECT_EQ(r1.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r1.new_status, ExchangeStatus::S2_SENT);

    const auto r2 = submit(DispatchFormType::S56, TelegramDirection::RECEIVED);
    EXPECT_EQ(r2.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r2.new_status, ExchangeStatus::S24_RECEIVED);
}

TEST_F(DispatchExchangeManagerFixture, CancellationPathS35)
{
    start_s2();

    const auto r = submit(DispatchFormType::S35, TelegramDirection::SENT);
    EXPECT_EQ(r.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r.new_status, ExchangeStatus::CANCELLED);
}

TEST_F(DispatchExchangeManagerFixture, NewExchangeAfterClosedGetsNewId)
{
    const auto r26 = progress_to_s26();
    manager.close(kSrcArea, kDstArea);

    const auto r2 = submit(DispatchFormType::S2, TelegramDirection::SENT, kNextTrain);
    EXPECT_EQ(r2.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r2.new_status, ExchangeStatus::S2_SENT);
    EXPECT_NE(r2.exchange_id, r26.exchange_id);
}

TEST_F(DispatchExchangeManagerFixture, NewExchangeAfterCancelled)
{
    start_s2();
    std::ignore = submit(DispatchFormType::S35, TelegramDirection::SENT);
    ASSERT_EQ(manager.status(kSrcArea, kDstArea), ExchangeStatus::CANCELLED);

    const auto r = submit(DispatchFormType::S2, TelegramDirection::SENT, kNextTrain);
    EXPECT_EQ(r.result, TelegramResult::ACCEPTED);
    EXPECT_EQ(r.new_status, ExchangeStatus::S2_SENT);
}

TEST_F(DispatchExchangeManagerFixture, DirectionsAreIndependent)
{
    start_s2();
    EXPECT_EQ(manager.status(kDstArea, kSrcArea), ExchangeStatus::IDLE);
}

TEST_F(DispatchExchangeManagerFixture, ExchangeIdsAreUniqueAcrossExchanges)
{
    const auto first = submit(DispatchFormType::S2, TelegramDirection::SENT);
    std::ignore = submit(DispatchFormType::S35, TelegramDirection::SENT);

    const auto second = submit(DispatchFormType::S2, TelegramDirection::SENT);
    EXPECT_NE(first.exchange_id, second.exchange_id);
    EXPECT_FALSE(second.exchange_id.empty());
}

struct DuplicateCase
{
    const char* name;
    SetupScenario setup;
    DispatchFormType form;
    TelegramDirection direction;
    ExchangeStatus expected_status;
    TelegramStatus expected_telegram_status;
};

class DispatchExchangeManagerDuplicateTest : public DispatchExchangeManagerFixture,
                                             public ::testing::WithParamInterface<DuplicateCase>
{
};

TEST_P(DispatchExchangeManagerDuplicateTest, RejectsDuplicateTelegram)
{
    const auto p = GetParam();
    prepare_state(p.setup);

    const auto r = submit(p.form, p.direction);
    EXPECT_EQ(r.result, TelegramResult::REJECTED_DUPLICATE);
    EXPECT_EQ(r.new_status, p.expected_status);
    EXPECT_EQ(r.telegram_status, p.expected_telegram_status);
}

INSTANTIATE_TEST_SUITE_P(
    DuplicateTelegrams, DispatchExchangeManagerDuplicateTest,
    ::testing::Values(DuplicateCase{"DuplicateS2", SetupScenario::S2Sent, DispatchFormType::S2,
                                    TelegramDirection::SENT, ExchangeStatus::S2_SENT,
                                    TelegramStatus::REJECTED},
                      DuplicateCase{"DuplicateS24", SetupScenario::S24Received,
                                    DispatchFormType::S24, TelegramDirection::RECEIVED,
                                    ExchangeStatus::S24_RECEIVED, TelegramStatus::REJECTED}),
    tests::common::param_name<DuplicateCase>);

struct WrongStateCase
{
    const char* name;
    SetupScenario setup;
    DispatchFormType form;
    TelegramDirection direction;
    ExchangeStatus expected_status;
};

class DispatchExchangeManagerWrongStateTest : public DispatchExchangeManagerFixture,
                                              public ::testing::WithParamInterface<WrongStateCase>
{
};

TEST_P(DispatchExchangeManagerWrongStateTest, RejectsOutOfOrderTelegram)
{
    const auto p = GetParam();
    prepare_state(p.setup);

    const auto r = submit(p.form, p.direction);
    EXPECT_EQ(r.result, TelegramResult::REJECTED_WRONG_STATE);
    EXPECT_EQ(r.new_status, p.expected_status);
}

INSTANTIATE_TEST_SUITE_P(
    WrongStateTelegrams, DispatchExchangeManagerWrongStateTest,
    ::testing::Values(WrongStateCase{"S24BeforeS2", SetupScenario::Idle, DispatchFormType::S24,
                                     TelegramDirection::RECEIVED, ExchangeStatus::IDLE},
                      WrongStateCase{"S25BeforeS24", SetupScenario::S2Sent, DispatchFormType::S25,
                                     TelegramDirection::SENT, ExchangeStatus::S2_SENT},
                      WrongStateCase{"S35FromIdle", SetupScenario::Idle, DispatchFormType::S35,
                                     TelegramDirection::SENT, ExchangeStatus::IDLE}),
    tests::common::param_name<WrongStateCase>);

struct CloseNoOpCase
{
    const char* name;
    SetupScenario setup;
    ExchangeStatus expected_status;
};

class DispatchExchangeManagerCloseNoOpTest : public DispatchExchangeManagerFixture,
                                             public ::testing::WithParamInterface<CloseNoOpCase>
{
};

TEST_P(DispatchExchangeManagerCloseNoOpTest, CloseDoesNotChangeNonTerminalState)
{
    const auto p = GetParam();
    prepare_state(p.setup);

    manager.close(kSrcArea, kDstArea);
    EXPECT_EQ(manager.status(kSrcArea, kDstArea), p.expected_status);
}

INSTANTIATE_TEST_SUITE_P(
    CloseNoOpStates, DispatchExchangeManagerCloseNoOpTest,
    ::testing::Values(CloseNoOpCase{"Idle", SetupScenario::Idle, ExchangeStatus::IDLE},
                      CloseNoOpCase{"S2Sent", SetupScenario::S2Sent, ExchangeStatus::S2_SENT}),
    tests::common::param_name<CloseNoOpCase>);

struct S51Case
{
    const char* name;
    SetupScenario setup;
    TelegramResult expected_result;
    ExchangeStatus expected_status;
};

class DispatchExchangeManagerS51Test : public DispatchExchangeManagerFixture,
                                       public ::testing::WithParamInterface<S51Case>
{
};

TEST_P(DispatchExchangeManagerS51Test, S51StateRules)
{
    const auto p = GetParam();
    prepare_state(p.setup);

    const auto r = submit(DispatchFormType::S51, TelegramDirection::SENT);
    EXPECT_EQ(r.result, p.expected_result);
    EXPECT_EQ(r.new_status, p.expected_status);
}

INSTANTIATE_TEST_SUITE_P(
    S51Cases, DispatchExchangeManagerS51Test,
    ::testing::Values(S51Case{"WhenActive", SetupScenario::S2Sent, TelegramResult::ACCEPTED,
                              ExchangeStatus::S2_SENT},
                      S51Case{"WhenIdle", SetupScenario::Idle, TelegramResult::REJECTED_WRONG_STATE,
                              ExchangeStatus::IDLE}),
    tests::common::param_name<S51Case>);

}  // namespace
