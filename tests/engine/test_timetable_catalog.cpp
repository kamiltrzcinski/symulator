#include <gtest/gtest.h>
#include "engine/timetable/timetable_catalog.hpp"

using namespace engine::core;
using namespace engine::timetable;

TEST(TimetableCatalogTest, ManagesPointsAndConnections) {
    TimetableCatalog catalog;

    TimetablePoint pt1;
    pt1.uid.value = 100;
    pt1.name = "Station A";
    pt1.point_type = TimetablePointType::STATION;

    catalog.add_point(pt1);

    EXPECT_TRUE(catalog.contains_point(UID{100}));
    EXPECT_FALSE(catalog.contains_point(UID{200}));

    auto found = catalog.find_point(UID{100});
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->name, "Station A");

    TimetableConnection conn;
    conn.uid.value = 300;
    conn.from_uid.value = 100;
    conn.to_uid.value = 200;

    catalog.add_connection(conn);

    EXPECT_TRUE(catalog.has_connections());
    EXPECT_TRUE(catalog.are_connected(UID{100}, UID{200}));
    EXPECT_FALSE(catalog.are_connected(UID{200}, UID{100}));
}
