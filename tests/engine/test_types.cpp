#include <gtest/gtest.h>

#include <engine/core/types.hpp>

using namespace engine::core;

// ── EntrySide ────────────────────────────────────────────────────────────────

TEST(EntrySide, DistinctValues)
{
    EXPECT_NE(EntrySide::LEFT, EntrySide::RIGHT);
}

// ── TrainSlot ────────────────────────────────────────────────────────────────

TEST(TrainSlot, DefaultInit)
{
    TrainSlot slot{};
    EXPECT_TRUE(slot.number.empty());
    EXPECT_FALSE(slot.has_extra_info);
    EXPECT_FALSE(slot.manually_placed);
    EXPECT_EQ(slot.entry_side, EntrySide::LEFT);
}

TEST(TrainSlot, FieldRoundtrip)
{
    TrainSlot slot{"IC1234", true, false, EntrySide::RIGHT};
    EXPECT_EQ(slot.number, "IC1234");
    EXPECT_TRUE(slot.has_extra_info);
    EXPECT_FALSE(slot.manually_placed);
    EXPECT_EQ(slot.entry_side, EntrySide::RIGHT);
}

TEST(TrainSlot, NumberSixChars)
{
    TrainSlot slot;
    slot.number = "ABC123";
    EXPECT_EQ(slot.number.size(), 6u);
}

TEST(TrainSlot, ManuallyPlaced)
{
    TrainSlot slot{"TLK567", false, true, EntrySide::LEFT};
    EXPECT_TRUE(slot.manually_placed);
    EXPECT_FALSE(slot.has_extra_info);
}

TEST(TrainSlot, EqualityOperator)
{
    TrainSlot a{"IC1234", false, false, EntrySide::LEFT};
    TrainSlot b{"IC1234", false, false, EntrySide::LEFT};
    TrainSlot c{"IC9999", false, false, EntrySide::LEFT};
    TrainSlot d{"IC1234", true, false, EntrySide::LEFT};
    TrainSlot e{"IC1234", false, false, EntrySide::RIGHT};

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

// ── PipEvent ─────────────────────────────────────────────────────────────────

TEST(PipEvent, SlotAbsent)
{
    auto section = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
    auto station = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
    PipEvent ev{section, station, TrackOccupancy::FREE, std::nullopt, false};
    EXPECT_EQ(ev.occupancy, TrackOccupancy::FREE);
    EXPECT_FALSE(ev.slot.has_value());
    EXPECT_FALSE(ev.lcs_boundary_crossing);
}

TEST(PipEvent, SlotPresent)
{
    auto section = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
    auto station = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
    TrainSlot slot{"IC1234", false, false, EntrySide::LEFT};
    PipEvent ev{section, station, TrackOccupancy::OCCUPIED, slot, false};
    ASSERT_TRUE(ev.slot.has_value());
    EXPECT_EQ(ev.slot->number, "IC1234");
    EXPECT_EQ(ev.slot->entry_side, EntrySide::LEFT);
    EXPECT_EQ(ev.occupancy, TrackOccupancy::OCCUPIED);
}

TEST(PipEvent, LcsBoundaryCrossing)
{
    auto section = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 10);
    auto station = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
    TrainSlot slot{"TLK567", false, false, EntrySide::RIGHT};
    PipEvent ev{section, station, TrackOccupancy::OCCUPIED, slot, true};
    EXPECT_TRUE(ev.lcs_boundary_crossing);
    ASSERT_TRUE(ev.slot.has_value());
    EXPECT_EQ(ev.slot->number, "TLK567");
    EXPECT_EQ(ev.slot->entry_side, EntrySide::RIGHT);
}

TEST(PipEvent, SectionAndStationUids)
{
    // Station GOr = instance 1, scope 1; Track section instance 42 on GOr
    auto section = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 42);
    auto station = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
    PipEvent ev{section, station, TrackOccupancy::FREE, std::nullopt, false};
    EXPECT_EQ(uid_kind(ev.section_uid), UIDKind::TRACK_SECTION);
    EXPECT_EQ(uid_scope(ev.section_uid), 1);
    EXPECT_EQ(uid_instance(ev.section_uid), 42);
    EXPECT_EQ(uid_kind(ev.station_uid), UIDKind::STATION);
    EXPECT_EQ(uid_scope(ev.station_uid), 1);
}

TEST(PipEvent, ExtraInfoFlag)
{
    auto section = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 3, 1);
    auto station = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 3, 1);
    TrainSlot slot{"EIC001", true, false, EntrySide::LEFT};
    PipEvent ev{section, station, TrackOccupancy::OCCUPIED, slot, false};
    ASSERT_TRUE(ev.slot.has_value());
    EXPECT_TRUE(ev.slot->has_extra_info);
}

TEST(PipEvent, SectionAndStationAreDifferentKinds)
{
    auto section = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
    auto station = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
    EXPECT_NE(section.value, station.value);
    EXPECT_TRUE(uid_has_kind(section, UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION));
    EXPECT_TRUE(uid_has_kind(station, UIDDomain::INFRASTRUCTURE, UIDKind::STATION));
}

// ── TrainCategory ────────────────────────────────────────────────────────────

TEST(TrainCategory, DistinctValues)
{
    EXPECT_NE(TrainCategory::PASSENGER, TrainCategory::FREIGHT);
    EXPECT_NE(TrainCategory::FREIGHT, TrainCategory::MAINTENANCE);
    EXPECT_NE(TrainCategory::PASSENGER, TrainCategory::MAINTENANCE);
}

TEST(TrainCategory, AllValues)
{
    auto cat = TrainCategory::PASSENGER;
    cat = TrainCategory::FREIGHT;
    cat = TrainCategory::MAINTENANCE;
    EXPECT_EQ(cat, TrainCategory::MAINTENANCE);
}

// ── DispatchFormType ─────────────────────────────────────────────────────────

TEST(DispatchFormType, DistinctValues)
{
    EXPECT_NE(DispatchFormType::S2, DispatchFormType::S24);
    EXPECT_NE(DispatchFormType::S24, DispatchFormType::S25);
    EXPECT_NE(DispatchFormType::S25, DispatchFormType::S26);
    EXPECT_NE(DispatchFormType::S55, DispatchFormType::S56);
}

TEST(DispatchFormType, AllFormsReachable)
{
    auto f = DispatchFormType::S2;
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

// ── TelegramDirection ────────────────────────────────────────────────────────

TEST(TelegramDirection, DistinctValues)
{
    EXPECT_NE(TelegramDirection::SENT, TelegramDirection::RECEIVED);
}

// ── TelegramStatus ───────────────────────────────────────────────────────────

TEST(TelegramStatus, DistinctValues)
{
    EXPECT_NE(TelegramStatus::PENDING, TelegramStatus::CONFIRMED);
    EXPECT_NE(TelegramStatus::CONFIRMED, TelegramStatus::REJECTED);
    EXPECT_NE(TelegramStatus::REJECTED, TelegramStatus::SUPERSEDED);
}

// ── ExchangeStatus ───────────────────────────────────────────────────────────

TEST(ExchangeStatus, DistinctValues)
{
    EXPECT_NE(ExchangeStatus::IDLE, ExchangeStatus::S2_SENT);
    EXPECT_NE(ExchangeStatus::S2_SENT, ExchangeStatus::S24_RECEIVED);
    EXPECT_NE(ExchangeStatus::S24_RECEIVED, ExchangeStatus::S25_SENT);
    EXPECT_NE(ExchangeStatus::S25_SENT, ExchangeStatus::S26_RECEIVED);
    EXPECT_NE(ExchangeStatus::S26_RECEIVED, ExchangeStatus::CLOSED);
    EXPECT_NE(ExchangeStatus::CLOSED, ExchangeStatus::CANCELLED);
}

TEST(ExchangeStatus, StandardPath)
{
    ExchangeStatus s = ExchangeStatus::IDLE;
    EXPECT_EQ(s, ExchangeStatus::IDLE);
    s = ExchangeStatus::S2_SENT;
    EXPECT_EQ(s, ExchangeStatus::S2_SENT);
    s = ExchangeStatus::S24_RECEIVED;
    EXPECT_EQ(s, ExchangeStatus::S24_RECEIVED);
    s = ExchangeStatus::S25_SENT;
    EXPECT_EQ(s, ExchangeStatus::S25_SENT);
    s = ExchangeStatus::S26_RECEIVED;
    EXPECT_EQ(s, ExchangeStatus::S26_RECEIVED);
    s = ExchangeStatus::CLOSED;
    EXPECT_EQ(s, ExchangeStatus::CLOSED);
}

TEST(ExchangeStatus, CancellationPath)
{
    ExchangeStatus s = ExchangeStatus::S2_SENT;
    s = ExchangeStatus::CANCELLED;
    EXPECT_EQ(s, ExchangeStatus::CANCELLED);
}
