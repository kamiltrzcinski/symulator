// tests/server/test_pip_writer.cpp

#include "server/db_writer.hpp"
#include "server/pip_writer.hpp"

#include "engine/core/types.hpp"

#include <nlohmann/json.hpp>

#include <gtest/gtest.h>

using namespace server;
using namespace engine::core;

// ── helpers ───────────────────────────────────────────────────────────────────

static PipEvent make_free_event(const char* section_gid, const char* station_sid)
{
    PipEvent ev;
    ev.section_gid = GID{section_gid};
    ev.station_sid = SID{station_sid};
    ev.occupancy = TrackOccupancy::FREE;
    ev.slot = std::nullopt;
    ev.lcs_boundary_crossing = false;
    return ev;
}

static PipEvent make_occupied_event(const char* section_gid, const char* station_sid,
                                    TrainSlot slot, bool boundary = false)
{
    PipEvent ev;
    ev.section_gid = GID{section_gid};
    ev.station_sid = SID{station_sid};
    ev.occupancy = TrackOccupancy::OCCUPIED;
    ev.slot = std::move(slot);
    ev.lcs_boundary_crossing = boundary;
    return ev;
}

// ── FreeSection ───────────────────────────────────────────────────────────────

TEST(PipWriter, FreeSection_UpsertWithEmptyTrains)
{
    NullDbWriter db;
    PipWriter writer{db, "sess-001"};

    writer.on_pip_events({make_free_event("gid-sec-01", "SOP")});

    ASSERT_EQ(db.pip_upserts.size(), 1u);
    EXPECT_EQ(db.pip_upserts[0].section_gid, "gid-sec-01");
    EXPECT_EQ(db.pip_upserts[0].trains_json, "[]");
}

// ── OccupiedSection ───────────────────────────────────────────────────────────

TEST(PipWriter, OccupiedSection_UpsertWithTrainSlot)
{
    NullDbWriter db;
    PipWriter writer{db, "sess-002"};

    TrainSlot slot{"IC123", true, false, EntrySide::RIGHT};
    writer.on_pip_events({make_occupied_event("gid-sec-02", "GDO", slot)});

    ASSERT_EQ(db.pip_upserts.size(), 1u);
    EXPECT_EQ(db.pip_upserts[0].section_gid, "gid-sec-02");

    // Parse JSON and verify fields rather than doing a fragile string compare.
    auto j = nlohmann::json::parse(db.pip_upserts[0].trains_json);
    ASSERT_EQ(j.size(), 1u);
    EXPECT_EQ(j[0]["number"].get<std::string>(), "IC123");
    EXPECT_EQ(j[0]["has_extra_info"].get<bool>(), true);
    EXPECT_EQ(j[0]["manually_placed"].get<bool>(), false);
    EXPECT_EQ(j[0]["entry_side"].get<std::string>(), "RIGHT");
}

// ── LcsBoundaryCrossing ───────────────────────────────────────────────────────

TEST(PipWriter, LcsBoundaryCrossing_UpsertTargetSection)
{
    NullDbWriter db;
    PipWriter writer{db, "sess-003"};

    TrainSlot slot{"TLK7", false, true, EntrySide::LEFT};
    writer.on_pip_events({make_occupied_event("gid-boundary-sec", "SOP", slot, /*boundary=*/true)});

    ASSERT_EQ(db.pip_upserts.size(), 1u);
    EXPECT_EQ(db.pip_upserts[0].section_gid, "gid-boundary-sec");

    auto j = nlohmann::json::parse(db.pip_upserts[0].trains_json);
    ASSERT_EQ(j.size(), 1u);
    EXPECT_EQ(j[0]["number"].get<std::string>(), "TLK7");
    EXPECT_EQ(j[0]["manually_placed"].get<bool>(), true);
    EXPECT_EQ(j[0]["entry_side"].get<std::string>(), "LEFT");
}

// ── MultipleBatch ─────────────────────────────────────────────────────────────

TEST(PipWriter, MultipleBatch_UpsertAllSections)
{
    NullDbWriter db;
    PipWriter writer{db, "sess-004"};

    writer.on_pip_events({
        make_free_event("sec-A", "STA"),
        make_occupied_event("sec-B", "STB", TrainSlot{"REG55", false, false, EntrySide::LEFT}),
        make_free_event("sec-C", "STC"),
    });

    ASSERT_EQ(db.pip_upserts.size(), 3u);
    EXPECT_EQ(db.pip_upserts[0].section_gid, "sec-A");
    EXPECT_EQ(db.pip_upserts[0].trains_json, "[]");
    EXPECT_EQ(db.pip_upserts[1].section_gid, "sec-B");
    EXPECT_NE(db.pip_upserts[1].trains_json, "[]");
    EXPECT_EQ(db.pip_upserts[2].section_gid, "sec-C");
    EXPECT_EQ(db.pip_upserts[2].trains_json, "[]");
}
