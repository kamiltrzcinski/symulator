#include <trackview/core/attachment_resolver.hpp>

#include <gtest/gtest.h>

namespace
{

TEST(PathAttachmentResolver, ResolvesDistanceAndLateralOffset)
{
    const trackview::Path path{{2, 10}, {20, 10}};
    const trackview::TrackAttachment attachment{{1001}, trackview::TrackSide::A,
                                                 0.25, -2};
    const auto result = trackview::PathAttachmentResolver{}.resolve(path, attachment);
    EXPECT_DOUBLE_EQ(result.x, 6.5);
    EXPECT_DOUBLE_EQ(result.y, 8.0);
}

TEST(PathAttachmentResolver, MeasuresOffsetFromSelectedSide)
{
    const trackview::Path path{{0, 0}, {100, 0}};
    const auto from_a = trackview::PathAttachmentResolver{}.resolve(
        path, {{1}, trackview::TrackSide::A, 0.1, 0});
    const auto from_b = trackview::PathAttachmentResolver{}.resolve(
        path, {{1}, trackview::TrackSide::B, 0.1, 0});
    EXPECT_DOUBLE_EQ(from_a.x, 10.0);
    EXPECT_DOUBLE_EQ(from_b.x, 90.0);
}

TEST(PathAttachmentResolver, RejectsZeroLengthPath)
{
    const trackview::Path path{{1, 1}, {1, 1}};
    EXPECT_THROW(static_cast<void>(trackview::PathAttachmentResolver{}.resolve(
                     path, {{1}, trackview::TrackSide::A, 0.5, 0})),
                 std::invalid_argument);
}

}  // namespace
