// tests/engine/test_spawn_resolver.cpp
//
// Unit tests for resolve_spawn_at_boundary (engine/core/spawn_resolver.hpp).
//
// Topology:
//
//   BND-A  ←[side_a]  OT-1  [side_b]→  OT-2  [side_b]→  BND-B
//   BND-AMB ← referenced by both OT-3 and OT-4 (ambiguous)
//   BND-LONELY ← referenced by no section

#include "engine/core/engine_state.hpp"
#include "engine/core/spawn_resolver.hpp"

#include <gtest/gtest.h>

#include <variant>

namespace
{

using namespace engine::core;

constexpr UID kBndA = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 1);
constexpr UID kBndB = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 2);
constexpr UID kBndAmb = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 3);
constexpr UID kBndLonely = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 4);
constexpr UID kOt1 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1);
constexpr UID kOt2 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 2);
constexpr UID kOt3 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 3);
constexpr UID kOt4 = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 4);
constexpr UID kSta = make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1);

TrackSection make_section(UID uid, UID side_a_neighbor, UID side_b_neighbor)
{
    TrackSection ts{};
    ts.uid = uid;
    ts.station_uid = kSta;
    ts.length_m = 100.0f;
    ts.side_a.neighbor_uid = side_a_neighbor;
    ts.side_b.neighbor_uid = side_b_neighbor;
    return ts;
}

EngineState make_state()
{
    EngineState s;
    s.set_session_id("test");
    s.insert_boundary_node(BoundaryNode{kBndA, "bnd-a", kSta, ""});
    s.insert_boundary_node(BoundaryNode{kBndB, "bnd-b", kSta, ""});
    s.insert_boundary_node(BoundaryNode{kBndAmb, "bnd-amb", kSta, ""});
    s.insert_boundary_node(BoundaryNode{kBndLonely, "bnd-lonely", kSta, ""});
    s.insert_track_section(make_section(kOt1, kBndA, kOt2));
    s.insert_track_section(make_section(kOt2, kOt1, kBndB));
    s.insert_track_section(make_section(kOt3, kBndAmb, UID{}));
    s.insert_track_section(make_section(kOt4, kBndAmb, UID{}));
    return s;
}

TEST(SpawnResolver, HappyPath_ReturnsAdjacentSectionAndBoundaryAsFrom)
{
    const auto state = make_state();
    const auto result = resolve_spawn_at_boundary(state, kBndA);

    const auto* point = std::get_if<SpawnPoint>(&result);
    ASSERT_NE(point, nullptr);
    EXPECT_EQ(point->section_uid, kOt1);
    EXPECT_EQ(point->from_uid, kBndA);
}

TEST(SpawnResolver, UnknownBoundary)
{
    const auto state = make_state();
    const auto result = resolve_spawn_at_boundary(
        state, make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 9, 9));

    const auto* error = std::get_if<SpawnError>(&result);
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(*error, SpawnError::UNKNOWN_BOUNDARY);
}

TEST(SpawnResolver, NoAdjacentSection)
{
    const auto state = make_state();
    const auto result = resolve_spawn_at_boundary(state, kBndLonely);

    const auto* error = std::get_if<SpawnError>(&result);
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(*error, SpawnError::NO_ADJACENT_SECTION);
}

TEST(SpawnResolver, AmbiguousBoundary)
{
    const auto state = make_state();
    const auto result = resolve_spawn_at_boundary(state, kBndAmb);

    const auto* error = std::get_if<SpawnError>(&result);
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(*error, SpawnError::AMBIGUOUS);
}

TEST(SpawnResolver, OccupiedSection)
{
    auto state = make_state();
    state.apply_track_section_occupancy(kOt1, TrackOccupancy::OCCUPIED, 4);
    const auto result = resolve_spawn_at_boundary(state, kBndA);

    const auto* error = std::get_if<SpawnError>(&result);
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(*error, SpawnError::SECTION_OCCUPIED);
}

TEST(SpawnResolver, ErrorMessagesAreHumanReadable)
{
    EXPECT_STRNE(to_string(SpawnError::UNKNOWN_BOUNDARY), "");
    EXPECT_STRNE(to_string(SpawnError::NO_ADJACENT_SECTION), "");
    EXPECT_STRNE(to_string(SpawnError::AMBIGUOUS), "");
    EXPECT_STRNE(to_string(SpawnError::SECTION_OCCUPIED), "");
}

}  // namespace
