// tests/server/test_pip_writer.cpp

#include "server/db_writer.hpp"
#include "server/pip_writer.hpp"

#include "engine/core/types.hpp"

#include <nlohmann/json.hpp>

#include <gtest/gtest.h>

using namespace server;
using namespace engine::core;

// ── helpers ───────────────────────────────────────────────────────────────────

static UID section(std::uint16_t n)
{
    return make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, n);
}

static UID station_1()
{
    return make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);
}

static PipEvent make_free_event(UID sec_uid)
{
    PipEvent ev;
    ev.section_uid = sec_uid;
    ev.station_uid = station_1();
    ev.occupancy = TrackOccupancy::FREE;
    ev.slot = std::nullopt;
    ev.lcs_boundary_crossing = false;
    return ev;
}

static PipEvent make_occupied_event(UID sec_uid, TrainSlot slot, bool boundary = false)
{
    PipEvent ev;
    ev.section_uid = sec_uid;
    ev.station_uid = station_1();
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
    const UID sec = section(1);

    writer.on_pip_events({make_free_event(sec)});

    ASSERT_EQ(db.pip_upserts.size(), 1u);
    EXPECT_EQ(db.pip_upserts[0].section_gid, std::to_string(sec.value));
    EXPECT_EQ(db.pip_upserts[0].trains_json, "[]");
}

// ── OccupiedSection ───────────────────────────────────────────────────────────

TEST(PipWriter, OccupiedSection_UpsertWithTrainSlot)
{
    NullDbWriter db;
    PipWriter writer{db, "sess-002"};
    const UID sec = section(2);

    TrainSlot slot{"IC123", true, false, EntrySide::RIGHT};
    writer.on_pip_events({make_occupied_event(sec, slot)});

    ASSERT_EQ(db.pip_upserts.size(), 1u);
    EXPECT_EQ(db.pip_upserts[0].section_gid, std::to_string(sec.value));

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
    const UID sec = section(3);

    TrainSlot slot{"TLK7", false, true, EntrySide::LEFT};
    writer.on_pip_events({make_occupied_event(sec, slot, /*boundary=*/true)});

    ASSERT_EQ(db.pip_upserts.size(), 1u);
    EXPECT_EQ(db.pip_upserts[0].section_gid, std::to_string(sec.value));

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
    const UID secA = section(10);
    const UID secB = section(11);
    const UID secC = section(12);

    writer.on_pip_events({
        make_free_event(secA),
        make_occupied_event(secB, TrainSlot{"REG55", false, false, EntrySide::LEFT}),
        make_free_event(secC),
    });

    ASSERT_EQ(db.pip_upserts.size(), 3u);
    EXPECT_EQ(db.pip_upserts[0].section_gid, std::to_string(secA.value));
    EXPECT_EQ(db.pip_upserts[0].trains_json, "[]");
    EXPECT_EQ(db.pip_upserts[1].section_gid, std::to_string(secB.value));
    EXPECT_NE(db.pip_upserts[1].trains_json, "[]");
    EXPECT_EQ(db.pip_upserts[2].section_gid, std::to_string(secC.value));
    EXPECT_EQ(db.pip_upserts[2].trains_json, "[]");
}
