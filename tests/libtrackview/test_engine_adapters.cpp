#include <engine/core/engine_state.hpp>

#include <trackview/adapters/engine_track_state_adapter.hpp>

#include <gtest/gtest.h>

namespace
{

engine::core::EngineState engine_state()
{
    engine::core::EngineState state;
    engine::core::TrackSection track;
    track.uid = {1001};
    track.occupancy = engine::core::TrackOccupancy::OCCUPIED;
    state.insert_track_section(track);

    engine::core::Switch switch_state;
    switch_state.uid = {2001};
    switch_state.position = engine::core::SwitchPosition::DIVERGENT;
    state.insert_switch(switch_state);

    engine::core::Signal signal_state;
    signal_state.uid = {3001};
    signal_state.governs_section_uid = {1001};
    signal_state.current_aspect = engine::core::SignalAspect::S2_PROCEED;
    state.insert_signal(signal_state);
    return state;
}

TEST(EngineInfrastructureCatalogAdapter, ExposesOnlyTrackViewTopologyQueries)
{
    const auto state = engine_state();
    const trackview::EngineInfrastructureCatalogAdapter catalog(state);
    EXPECT_TRUE(catalog.contains_track({1001}));
    EXPECT_TRUE(catalog.contains_switch({2001}));
    EXPECT_TRUE(catalog.contains_signal({3001}));
    EXPECT_TRUE(catalog.signal_governs_track({3001}, {1001}));
    EXPECT_FALSE(catalog.contains_track({9999}));
}

TEST(EngineTrackRuntimeAdapter, MapsEngineEnumsToStableViewVocabulary)
{
    const auto state = engine_state();
    const trackview::EngineTrackRuntimeAdapter runtime(state);
    ASSERT_TRUE(runtime.track_state({1001}).has_value());
    ASSERT_TRUE(runtime.switch_state({2001}).has_value());
    ASSERT_TRUE(runtime.signal_state({3001}).has_value());
    EXPECT_EQ(runtime.track_state({1001})->occupancy,
              trackview::OccupancyState::Occupied);
    EXPECT_EQ(runtime.switch_state({2001})->position,
              trackview::SwitchPositionState::Divergent);
    EXPECT_EQ(runtime.signal_state({3001})->indication,
              trackview::SignalIndicationState::Proceed);
}

}  // namespace
