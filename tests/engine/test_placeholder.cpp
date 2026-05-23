#include <gtest/gtest.h>

#include <engine/core/engine_state.hpp>
#include <engine/core/track_model.hpp>

using namespace engine::core;

// ── TrackSection insert / find round-trip ────────────────────────────────────

TEST(EngineState, InsertAndFindTrackSection)
{
    EngineState state;

    TrackSection ts;
    ts.gid = GID{"OT-TEST-001"};
    ts.pid = "ot_test_001";
    ts.length_m = 250.0f;
    state.insert_track_section(ts);

    const TrackSection* found = state.find_track_section(GID{"OT-TEST-001"});
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->gid.value, "OT-TEST-001");
    EXPECT_FLOAT_EQ(found->length_m, 250.0f);
}

TEST(EngineState, FindTrackSectionMissing)
{
    EngineState state;
    EXPECT_EQ(state.find_track_section(GID{"NONEXISTENT"}), nullptr);
}

// ── Switch insert / find round-trip ─────────────────────────────────────────

TEST(EngineState, InsertAndFindSwitch)
{
    EngineState state;

    Switch sw;
    sw.gid = GID{"ZWR-TEST-001"};
    sw.pid = "zwr_test_001";
    sw.position = SwitchPosition::STRAIGHT;
    state.insert_switch(sw);

    const Switch* found = state.find_switch(GID{"ZWR-TEST-001"});
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->gid.value, "ZWR-TEST-001");
    EXPECT_EQ(found->position, SwitchPosition::STRAIGHT);
}

TEST(EngineState, FindSwitchMissing)
{
    EngineState state;
    EXPECT_EQ(state.find_switch(GID{"NONEXISTENT"}), nullptr);
}
