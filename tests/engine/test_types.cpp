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
    PipEvent ev{GID{"OT-GOr-tor_1a-0000001"}, SID{"GOr"}, TrackOccupancy::FREE, std::nullopt,
                false};
    EXPECT_EQ(ev.occupancy, TrackOccupancy::FREE);
    EXPECT_FALSE(ev.slot.has_value());
    EXPECT_FALSE(ev.lcs_boundary_crossing);
}

TEST(PipEvent, SlotPresent)
{
    TrainSlot slot{"IC1234", false, false, EntrySide::LEFT};
    PipEvent ev{GID{"OT-GOr-tor_1a-0000001"}, SID{"GOr"}, TrackOccupancy::OCCUPIED, slot, false};
    ASSERT_TRUE(ev.slot.has_value());
    EXPECT_EQ(ev.slot->number, "IC1234");
    EXPECT_EQ(ev.slot->entry_side, EntrySide::LEFT);
    EXPECT_EQ(ev.occupancy, TrackOccupancy::OCCUPIED);
}

TEST(PipEvent, LcsBoundaryCrossing)
{
    TrainSlot slot{"TLK567", false, false, EntrySide::RIGHT};
    PipEvent ev{GID{"OT-GOr-szlak_1-0000001"}, SID{"GOr"}, TrackOccupancy::OCCUPIED, slot, true};
    EXPECT_TRUE(ev.lcs_boundary_crossing);
    ASSERT_TRUE(ev.slot.has_value());
    EXPECT_EQ(ev.slot->number, "TLK567");
    EXPECT_EQ(ev.slot->entry_side, EntrySide::RIGHT);
}

TEST(PipEvent, SectionAndStationIds)
{
    PipEvent ev{GID{"OT-GGO-tor_2-0000042"}, SID{"GGO"}, TrackOccupancy::FREE, std::nullopt, false};
    EXPECT_EQ(ev.section_gid.value, "OT-GGO-tor_2-0000042");
    EXPECT_EQ(ev.station_sid.value, "GGO");
}

TEST(PipEvent, ExtraInfoFlag)
{
    TrainSlot slot{"EIC001", true, false, EntrySide::LEFT};
    PipEvent ev{GID{"OT-GGO-tor_1-0000001"}, SID{"GGO"}, TrackOccupancy::OCCUPIED, slot, false};
    ASSERT_TRUE(ev.slot.has_value());
    EXPECT_TRUE(ev.slot->has_extra_info);
}
