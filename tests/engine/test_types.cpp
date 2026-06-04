#include <gtest/gtest.h>

#include <engine/core/types.hpp>

#include <tests/common/param_test_helpers.hpp>

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace
{

using namespace engine::core;

// ── EntrySide ────────────────────────────────────────────────────────────────

struct EntrySideDistinctCase
{
    const char* name;
    EntrySide lhs;
    EntrySide rhs;
};

class EntrySideDistinctTest : public ::testing::TestWithParam<EntrySideDistinctCase>
{
};

TEST_P(EntrySideDistinctTest, DistinctValues)
{
    const auto& p = GetParam();
    EXPECT_NE(p.lhs, p.rhs);
}

INSTANTIATE_TEST_SUITE_P(EntrySideCases, EntrySideDistinctTest,
                         ::testing::Values(EntrySideDistinctCase{"LeftVsRight", EntrySide::LEFT,
                                                                 EntrySide::RIGHT}),
                         tests::common::param_name<EntrySideDistinctCase>);

// ── TrainSlot ────────────────────────────────────────────────────────────────

TEST(TrainSlot, DefaultInit)
{
    TrainSlot slot{};
    EXPECT_TRUE(slot.number.empty());
    EXPECT_FALSE(slot.has_extra_info);
    EXPECT_FALSE(slot.manually_placed);
    EXPECT_EQ(slot.entry_side, EntrySide::LEFT);
}

struct TrainSlotProjectionCase
{
    const char* name;
    TrainSlot slot;
    std::size_t expected_number_size;
    bool expected_has_extra;
    bool expected_manually_placed;
    EntrySide expected_side;
};

class TrainSlotProjectionTest : public ::testing::TestWithParam<TrainSlotProjectionCase>
{
};

TEST_P(TrainSlotProjectionTest, FieldProjection)
{
    const auto& p = GetParam();
    EXPECT_EQ(p.slot.number.size(), p.expected_number_size);
    EXPECT_EQ(p.slot.has_extra_info, p.expected_has_extra);
    EXPECT_EQ(p.slot.manually_placed, p.expected_manually_placed);
    EXPECT_EQ(p.slot.entry_side, p.expected_side);
}

INSTANTIATE_TEST_SUITE_P(
    TrainSlotCases, TrainSlotProjectionTest,
    ::testing::Values(TrainSlotProjectionCase{"Roundtrip",
                                              TrainSlot{"IC1234", true, false, EntrySide::RIGHT},
                                              6u, true, false, EntrySide::RIGHT},
                      TrainSlotProjectionCase{"NumberSixChars",
                                              TrainSlot{"ABC123", false, false, EntrySide::LEFT},
                                              6u, false, false, EntrySide::LEFT},
                      TrainSlotProjectionCase{"ManuallyPlaced",
                                              TrainSlot{"TLK567", false, true, EntrySide::LEFT}, 6u,
                                              false, true, EntrySide::LEFT}),
    tests::common::param_name<TrainSlotProjectionCase>);

TEST(TrainSlot, EqualityOperator)
{
    const TrainSlot a{"IC1234", false, false, EntrySide::LEFT};
    const TrainSlot b{"IC1234", false, false, EntrySide::LEFT};
    const TrainSlot c{"IC9999", false, false, EntrySide::LEFT};
    const TrainSlot d{"IC1234", true, false, EntrySide::LEFT};
    const TrainSlot e{"IC1234", false, false, EntrySide::RIGHT};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

// ── PipEvent ─────────────────────────────────────────────────────────────────

struct PipSlotCase
{
    const char* name;
    TrackOccupancy occupancy;
    std::optional<TrainSlot> slot;
    bool lcs_boundary_crossing;
    bool expect_slot;
    bool expect_extra_info;
    EntrySide expected_side;
};

class PipSlotProjectionTest : public ::testing::TestWithParam<PipSlotCase>
{
};

TEST_P(PipSlotProjectionTest, SlotProjection)
{
    const auto& p = GetParam();
    const auto section = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
    const auto station = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);

    const PipEvent ev{section, station, p.occupancy, p.slot, p.lcs_boundary_crossing};

    EXPECT_EQ(ev.occupancy, p.occupancy);
    EXPECT_EQ(ev.lcs_boundary_crossing, p.lcs_boundary_crossing);
    EXPECT_EQ(ev.slot.has_value(), p.expect_slot);

    if (p.expect_slot)
    {
        ASSERT_TRUE(ev.slot.has_value());
        EXPECT_EQ(ev.slot->has_extra_info, p.expect_extra_info);
        EXPECT_EQ(ev.slot->entry_side, p.expected_side);
    }
}

INSTANTIATE_TEST_SUITE_P(
    PipSlotCases, PipSlotProjectionTest,
    ::testing::Values(PipSlotCase{"SlotAbsent", TrackOccupancy::FREE, std::nullopt, false, false,
                                  false, EntrySide::LEFT},
                      PipSlotCase{"SlotPresent", TrackOccupancy::OCCUPIED,
                                  TrainSlot{"IC1234", false, false, EntrySide::LEFT}, false, true,
                                  false, EntrySide::LEFT},
                      PipSlotCase{"BoundaryCrossingWithExtraInfo", TrackOccupancy::OCCUPIED,
                                  TrainSlot{"TLK567", true, false, EntrySide::RIGHT}, true, true,
                                  true, EntrySide::RIGHT}),
    tests::common::param_name<PipSlotCase>);

TEST(PipEvent, SectionAndStationUids)
{
    const auto section = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 42);
    const auto station = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
    const PipEvent ev{section, station, TrackOccupancy::FREE, std::nullopt, false};

    EXPECT_EQ(uid_kind(ev.section_uid), UIDKind::TRACK_SECTION);
    EXPECT_EQ(uid_scope(ev.section_uid), 1);
    EXPECT_EQ(uid_instance(ev.section_uid), 42);
    EXPECT_EQ(uid_kind(ev.station_uid), UIDKind::STATION);
    EXPECT_EQ(uid_scope(ev.station_uid), 1);
}

TEST(PipEvent, SectionAndStationAreDifferentKinds)
{
    const auto section = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
    const auto station = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);

    EXPECT_NE(section.value, station.value);
    EXPECT_TRUE(uid_has_kind(section, UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION));
    EXPECT_TRUE(uid_has_kind(station, UIDDomain::INFRASTRUCTURE, UIDKind::STATION));
}

// ── TrainCategory / Dispatch / Exchange enums ───────────────────────────────

template<typename EnumT>
struct EnumDistinctCase
{
    const char* name;
    EnumT lhs;
    EnumT rhs;
};

class TrainCategoryDistinctTest : public ::testing::TestWithParam<EnumDistinctCase<TrainCategory>>
{
};

TEST_P(TrainCategoryDistinctTest, DistinctValues)
{
    const auto& p = GetParam();
    EXPECT_NE(p.lhs, p.rhs);
}

INSTANTIATE_TEST_SUITE_P(TrainCategoryDistinct, TrainCategoryDistinctTest,
                         ::testing::Values(EnumDistinctCase<TrainCategory>{"PassengerVsFreight",
                                                                           TrainCategory::PASSENGER,
                                                                           TrainCategory::FREIGHT},
                                           EnumDistinctCase<TrainCategory>{
                                               "FreightVsMaintenance", TrainCategory::FREIGHT,
                                               TrainCategory::MAINTENANCE},
                                           EnumDistinctCase<TrainCategory>{
                                               "PassengerVsMaintenance", TrainCategory::PASSENGER,
                                               TrainCategory::MAINTENANCE}),
                         tests::common::param_name<EnumDistinctCase<TrainCategory>>);

class DispatchFormDistinctTest : public ::testing::TestWithParam<EnumDistinctCase<DispatchFormType>>
{
};

TEST_P(DispatchFormDistinctTest, DistinctValues)
{
    const auto& p = GetParam();
    EXPECT_NE(p.lhs, p.rhs);
}

INSTANTIATE_TEST_SUITE_P(
    DispatchFormDistinct, DispatchFormDistinctTest,
    ::testing::Values(EnumDistinctCase<DispatchFormType>{"S2VsS24", DispatchFormType::S2,
                                                         DispatchFormType::S24},
                      EnumDistinctCase<DispatchFormType>{"S24VsS25", DispatchFormType::S24,
                                                         DispatchFormType::S25},
                      EnumDistinctCase<DispatchFormType>{"S25VsS26", DispatchFormType::S25,
                                                         DispatchFormType::S26},
                      EnumDistinctCase<DispatchFormType>{"S55VsS56", DispatchFormType::S55,
                                                         DispatchFormType::S56}),
    tests::common::param_name<EnumDistinctCase<DispatchFormType>>);

class TelegramDirectionDistinctTest
    : public ::testing::TestWithParam<EnumDistinctCase<TelegramDirection>>
{
};

TEST_P(TelegramDirectionDistinctTest, DistinctValues)
{
    const auto& p = GetParam();
    EXPECT_NE(p.lhs, p.rhs);
}

INSTANTIATE_TEST_SUITE_P(TelegramDirectionDistinct, TelegramDirectionDistinctTest,
                         ::testing::Values(EnumDistinctCase<TelegramDirection>{
                             "SentVsReceived", TelegramDirection::SENT,
                             TelegramDirection::RECEIVED}),
                         tests::common::param_name<EnumDistinctCase<TelegramDirection>>);

class TelegramStatusDistinctTest : public ::testing::TestWithParam<EnumDistinctCase<TelegramStatus>>
{
};

TEST_P(TelegramStatusDistinctTest, DistinctValues)
{
    const auto& p = GetParam();
    EXPECT_NE(p.lhs, p.rhs);
}

INSTANTIATE_TEST_SUITE_P(
    TelegramStatusDistinct, TelegramStatusDistinctTest,
    ::testing::Values(
        EnumDistinctCase<TelegramStatus>{"PendingVsConfirmed", TelegramStatus::PENDING,
                                         TelegramStatus::CONFIRMED},
        EnumDistinctCase<TelegramStatus>{"ConfirmedVsRejected", TelegramStatus::CONFIRMED,
                                         TelegramStatus::REJECTED},
        EnumDistinctCase<TelegramStatus>{"RejectedVsSuperseded", TelegramStatus::REJECTED,
                                         TelegramStatus::SUPERSEDED}),
    tests::common::param_name<EnumDistinctCase<TelegramStatus>>);

class ExchangeStatusDistinctTest : public ::testing::TestWithParam<EnumDistinctCase<ExchangeStatus>>
{
};

TEST_P(ExchangeStatusDistinctTest, DistinctValues)
{
    const auto& p = GetParam();
    EXPECT_NE(p.lhs, p.rhs);
}

INSTANTIATE_TEST_SUITE_P(
    ExchangeStatusDistinct, ExchangeStatusDistinctTest,
    ::testing::Values(EnumDistinctCase<ExchangeStatus>{"IdleVsS2", ExchangeStatus::IDLE,
                                                       ExchangeStatus::S2_SENT},
                      EnumDistinctCase<ExchangeStatus>{"S2VsS24", ExchangeStatus::S2_SENT,
                                                       ExchangeStatus::S24_RECEIVED},
                      EnumDistinctCase<ExchangeStatus>{"S24VsS25", ExchangeStatus::S24_RECEIVED,
                                                       ExchangeStatus::S25_SENT},
                      EnumDistinctCase<ExchangeStatus>{"S25VsS26", ExchangeStatus::S25_SENT,
                                                       ExchangeStatus::S26_RECEIVED},
                      EnumDistinctCase<ExchangeStatus>{"S26VsClosed", ExchangeStatus::S26_RECEIVED,
                                                       ExchangeStatus::CLOSED},
                      EnumDistinctCase<ExchangeStatus>{"ClosedVsCancelled", ExchangeStatus::CLOSED,
                                                       ExchangeStatus::CANCELLED}),
    tests::common::param_name<EnumDistinctCase<ExchangeStatus>>);

struct EnumSequenceCase
{
    const char* name;
    std::vector<ExchangeStatus> sequence;
    ExchangeStatus expected_final;
};

class ExchangeStatusSequenceTest : public ::testing::TestWithParam<EnumSequenceCase>
{
};

TEST_P(ExchangeStatusSequenceTest, PathEndsAtExpectedState)
{
    const auto& p = GetParam();
    ASSERT_FALSE(p.sequence.empty());
    ExchangeStatus s = p.sequence.front();
    for (std::size_t i = 1; i < p.sequence.size(); ++i)
    {
        s = p.sequence[i];
    }
    EXPECT_EQ(s, p.expected_final);
}

INSTANTIATE_TEST_SUITE_P(
    ExchangeStatusPaths, ExchangeStatusSequenceTest,
    ::testing::Values(EnumSequenceCase{"StandardPath",
                                       {ExchangeStatus::IDLE, ExchangeStatus::S2_SENT,
                                        ExchangeStatus::S24_RECEIVED, ExchangeStatus::S25_SENT,
                                        ExchangeStatus::S26_RECEIVED, ExchangeStatus::CLOSED},
                                       ExchangeStatus::CLOSED},
                      EnumSequenceCase{"CancellationPath",
                                       {ExchangeStatus::S2_SENT, ExchangeStatus::CANCELLED},
                                       ExchangeStatus::CANCELLED}),
    tests::common::param_name<EnumSequenceCase>);

TEST(DispatchFormType, AllFormsReachable)
{
    DispatchFormType f = DispatchFormType::S2;
    f = DispatchFormType::S24;
    f = DispatchFormType::S25;
    f = DispatchFormType::S26;
    f = DispatchFormType::S35;
    f = DispatchFormType::S51;
    f = DispatchFormType::S52;
    f = DispatchFormType::S55;
    f = DispatchFormType::S56;
    f = DispatchFormType::S76;
    EXPECT_EQ(f, DispatchFormType::S76);
}

TEST(TrainCategory, AllValues)
{
    TrainCategory cat = TrainCategory::PASSENGER;
    cat = TrainCategory::FREIGHT;
    cat = TrainCategory::MAINTENANCE;
    EXPECT_EQ(cat, TrainCategory::MAINTENANCE);
}

}  // namespace
