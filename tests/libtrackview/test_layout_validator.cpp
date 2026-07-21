#include "fakes.hpp"

#include <trackview/core/layout_validator.hpp>

#include <gtest/gtest.h>

#include <algorithm>

namespace
{

trackview::TrackLayout valid_layout()
{
    trackview::TrackLayout layout;
    layout.layout_id = "test";
    layout.station_sid = "TEST";
    layout.canvas = {40, 20};
    layout.elements.emplace_back(
        trackview::TrackSectionGeometry{{1001}, {{0, 10}, {20, 10}}});
    layout.elements.emplace_back(trackview::SwitchGeometry{
        {2001}, {{20, 10}, {30, 10}, {30, 15}}});
    layout.elements.emplace_back(trackview::SignalGeometry{
        {3001}, {{1001}, trackview::TrackSide::A, 0.25, -2},
        trackview::FacingDirection::TowardsB});
    return layout;
}

trackview::test::FakeCatalog valid_catalog()
{
    trackview::test::FakeCatalog catalog;
    catalog.tracks.insert({1001});
    catalog.switches.insert({2001});
    catalog.signal_items.insert({3001});
    catalog.governed_tracks.emplace(trackview::InfrastructureId{3001},
                                    trackview::InfrastructureId{1001});
    return catalog;
}

TEST(LayoutValidator, AcceptsConsistentLayout)
{
    EXPECT_TRUE(trackview::LayoutValidator{}.validate(valid_catalog(), valid_layout()).empty());
}

TEST(LayoutValidator, ReportsUnknownTrackThroughSegregatedCatalog)
{
    auto catalog = valid_catalog();
    catalog.tracks.clear();
    const auto diagnostics =
        trackview::LayoutValidator{}.validate(catalog, valid_layout());
    ASSERT_FALSE(diagnostics.empty());
    EXPECT_EQ(diagnostics.front().rule_id, "LAY-101");
}

TEST(LayoutValidator, RejectsSignalAttachedToTrackItDoesNotGovern)
{
    auto catalog = valid_catalog();
    catalog.governed_tracks.at({3001}) = {9999};
    const auto diagnostics =
        trackview::LayoutValidator{}.validate(catalog, valid_layout());
    EXPECT_TRUE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& item) {
        return item.rule_id == "LAY-303";
    }));
}

TEST(LayoutValidator, RejectsDuplicateGeometryAndInvalidOffset)
{
    auto layout = valid_layout();
    layout.elements.push_back(layout.elements.front());
    auto& signal = std::get<trackview::SignalGeometry>(layout.elements[2]);
    signal.attachment.offset = 1.5;
    const auto diagnostics =
        trackview::LayoutValidator{}.validate(valid_catalog(), layout);
    EXPECT_TRUE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& item) {
        return item.rule_id == "LAY-104";
    }));
    EXPECT_TRUE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& item) {
        return item.rule_id == "LAY-304";
    }));
}

}  // namespace
