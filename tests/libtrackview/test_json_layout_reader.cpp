#include <trackview/core/layout_reader.hpp>

#include <gtest/gtest.h>

#include <filesystem>

namespace
{

TEST(JsonLayoutReader, ReadsAllTrackSchematicElementTypes)
{
    const auto layout = trackview::JsonLayoutReader{}.read(
        std::filesystem::path(TRACKVIEW_FIXTURE_DIR) / "simple_layout.json");
    EXPECT_EQ(layout.schema_version, 1);
    EXPECT_EQ(layout.layout_id, "simple-ebiscreen");
    EXPECT_EQ(layout.station_sid, "TEST");
    ASSERT_EQ(layout.elements.size(), 3U);
    EXPECT_TRUE(std::holds_alternative<trackview::TrackSectionGeometry>(layout.elements[0]));
    EXPECT_TRUE(std::holds_alternative<trackview::SwitchGeometry>(layout.elements[1]));
    EXPECT_TRUE(std::holds_alternative<trackview::SignalGeometry>(layout.elements[2]));
}

TEST(JsonLayoutReader, ReportsMissingFile)
{
    EXPECT_THROW(static_cast<void>(trackview::JsonLayoutReader{}.read("does-not-exist.json")),
                 std::runtime_error);
}

}  // namespace
