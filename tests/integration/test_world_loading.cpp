// tests/integration/test_world_loading.cpp
//
// Integration tests for engine::core::load_world() — loading multiple
// station scenarios into a single EngineState, and verifying that their
// cross-referenced trunk-line ("szlak") sections resolve to each other.
//
// Loads real scenario data from the repository tree (scenarios/). Does not
// require a PostgreSQL instance.

#include <gtest/gtest.h>

#include <engine/core/engine_state.hpp>
#include <engine/core/topology_loader.hpp>
#include <engine/core/types.hpp>

#include <filesystem>
#include <vector>

using namespace engine::core;
namespace fs = std::filesystem;

// Path to repository root — injected by CMake.
#ifndef REPO_ROOT
#error "REPO_ROOT must be defined by CMake"
#endif

namespace
{

std::vector<fs::path> gor_and_sopot_dirs()
{
    return {
        fs::path(REPO_ROOT) / "scenarios" / "gdynia_orlowo",
        fs::path(REPO_ROOT) / "scenarios" / "sopot",
    };
}

// UIDs of the L202 trunk-line sections that cross-reference each other
// between Sopot and Gdynia Orlowo (see docs/plan_UID.md Etap 2C).
constexpr UID kSopotL202 = UID{2280627765267};
constexpr UID kGorL202 = UID{2280627699729};

}  // namespace

TEST(WorldLoading, LoadsBothStationsWithoutThrowing)
{
    EngineState state;
    EXPECT_NO_THROW(load_world(state, gor_and_sopot_dirs()));
}

TEST(WorldLoading, FirstScenarioIsPrimaryForSessionId)
{
    EngineState state;
    load_world(state, gor_and_sopot_dirs());
    EXPECT_EQ(state.session_id(), "GOr");
}

TEST(WorldLoading, CrossReferencedTrunkSectionsResolveToEachOther)
{
    EngineState state;
    load_world(state, gor_and_sopot_dirs());

    const auto* sopot_section = state.find_track_section(kSopotL202);
    const auto* gor_section = state.find_track_section(kGorL202);
    ASSERT_NE(sopot_section, nullptr);
    ASSERT_NE(gor_section, nullptr);

    EXPECT_EQ(sopot_section->side_b.neighbor_uid, kGorL202);
    EXPECT_EQ(gor_section->side_a.neighbor_uid, kSopotL202);
}

TEST(WorldLoading, ThrowsOnEmptyScenarioList)
{
    EngineState state;
    EXPECT_THROW(load_world(state, {}), std::runtime_error);
}
