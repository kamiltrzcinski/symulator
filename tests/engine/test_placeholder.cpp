#include <gtest/gtest.h>

#include <engine/core/engine_state.hpp>
#include <engine/core/track_model.hpp>

using namespace engine::core;

// ── TrackSection insert / find round-trip ────────────────────────────────────

TEST(EngineState, InsertAndFindTrackSection)
{
    EngineState state;

    TrackSection ts;
    ts.uid = UID{0x020300000001ULL};
    ts.pid = "ot_test_001";
    ts.length_m = 250.0f;
    state.insert_track_section(ts);

    const TrackSection* found = state.find_track_section(UID{0x020300000001ULL});
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->uid.value, 0x020300000001ULL);
    EXPECT_FLOAT_EQ(found->length_m, 250.0f);
}

TEST(EngineState, FindTrackSectionMissing)
{
    EngineState state;
    EXPECT_EQ(state.find_track_section(UID{0xDEADBEEFULL}), nullptr);
}

// ── Switch insert / find round-trip ─────────────────────────────────────────

TEST(EngineState, InsertAndFindSwitch)
{
    EngineState state;

    Switch sw;
    sw.uid = UID{0x020400000001ULL};
    sw.pid = "zwr_test_001";
    sw.position = SwitchPosition::STRAIGHT;
    state.insert_switch(sw);

    const Switch* found = state.find_switch(UID{0x020400000001ULL});
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->uid.value, 0x020400000001ULL);
    EXPECT_EQ(found->position, SwitchPosition::STRAIGHT);
}

TEST(EngineState, FindSwitchMissing)
{
    EngineState state;
    EXPECT_EQ(state.find_switch(UID{0xDEADBEEFULL}), nullptr);
}
