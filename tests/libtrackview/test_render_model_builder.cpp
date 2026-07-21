#include "fakes.hpp"

#include <trackview/core/render_model_builder.hpp>

#include <gtest/gtest.h>

namespace
{

trackview::TrackLayout sample_layout()
{
    trackview::TrackLayout layout;
    layout.layout_id = "sample";
    layout.station_sid = "TEST";
    layout.canvas = {40, 20};
    layout.elements.emplace_back(
        trackview::TrackSectionGeometry{{1001}, {{2, 10}, {20, 10}}});
    layout.elements.emplace_back(trackview::SwitchGeometry{
        {2001}, {{20, 10}, {30, 10}, {30, 15}}});
    layout.elements.emplace_back(trackview::SignalGeometry{
        {3001}, {{1001}, trackview::TrackSide::A, 0.25, -2},
        trackview::FacingDirection::TowardsB});
    return layout;
}

trackview::test::FakeRuntimeState sample_state()
{
    trackview::test::FakeRuntimeState state;
    state.tracks.emplace(trackview::InfrastructureId{1001},
                         trackview::TrackRuntimeState{trackview::OccupancyState::Occupied});
    state.switches.emplace(
        trackview::InfrastructureId{2001},
        trackview::SwitchRuntimeState{trackview::OccupancyState::Free,
                                      trackview::SwitchPositionState::Divergent});
    state.signal_items.emplace(
        trackview::InfrastructureId{3001},
        trackview::SignalRuntimeState{trackview::SignalIndicationState::Stop});
    return state;
}

TEST(RenderModelBuilder, DependsOnSubstitutableAttachmentResolver)
{
    const trackview::test::FixedAttachmentResolver resolver({7, 9});
    const auto model =
        trackview::RenderModelBuilder(resolver).build(sample_state(), sample_layout());
    ASSERT_EQ(model.signal_items.size(), 1U);
    EXPECT_DOUBLE_EQ(model.signal_items.front().position.x, 7.0);
    EXPECT_DOUBLE_EQ(model.signal_items.front().position.y, 9.0);
}

TEST(RenderModelBuilder, MapsOnlyRuntimeStateRequiredByRendering)
{
    const trackview::PathAttachmentResolver resolver;
    const auto model =
        trackview::RenderModelBuilder(resolver).build(sample_state(), sample_layout());
    ASSERT_EQ(model.tracks.size(), 1U);
    ASSERT_EQ(model.switches.size(), 1U);
    EXPECT_EQ(model.tracks.front().state.occupancy, trackview::OccupancyState::Occupied);
    EXPECT_EQ(model.switches.front().state.position,
              trackview::SwitchPositionState::Divergent);
}

TEST(RenderModelBuilder, FailsExplicitlyWhenRequiredStateIsUnavailable)
{
    auto state = sample_state();
    state.tracks.clear();
    const trackview::PathAttachmentResolver resolver;
    EXPECT_THROW(static_cast<void>(
                     trackview::RenderModelBuilder(resolver).build(state, sample_layout())),
                 std::runtime_error);
}

}  // namespace
