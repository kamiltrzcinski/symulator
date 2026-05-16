#pragma once

#include <engine/core/control_system.hpp>
#include <engine/core/state_view.hpp>
#include <engine/core/track_model.hpp>
#include <engine/core/types.hpp>

#include <optional>
#include <unordered_map>
#include <vector>

// ── Route graph traversal ─────────────────────────────────────────────────────
// Finds an interlocking route path between two signals using BFS on the
// station topology graph.
//
// Node types:  TrackSection, Switch, BoundaryNode.
// BoundaryNode is a dead end in the graph (no outgoing edges from our side).
//
// The returned RoutePath describes:
//  - The ordered list of topology nodes (sections + switches)
//  - The required position for each switch on the path
//
// Returns nullopt when no path exists, or when a required node is blocked.

namespace srk::common
{

using namespace engine::core;

// One node in a found route path.
struct RoutePathNode
{
    enum class Kind : std::uint8_t
    {
        TRACK_SECTION,
        SWITCH
    } kind;
    GID gid;
    // Only meaningful when kind == SWITCH.
    SwitchPosition required_position = SwitchPosition::STRAIGHT;
};

struct RoutePath
{
    GID from_signal_gid;
    GID to_signal_gid;
    std::vector<RoutePathNode> nodes;
    std::vector<GID> section_gids;   // pre-extracted for convenience
    std::vector<GID> switch_gids;    // pre-extracted for convenience
    std::vector<GID> derailer_gids;  // derailers guarding sections on the path
};

// Find a route path from the entry signal to the exit signal.
// Returns nullopt if no path exists in the topology.
std::optional<RoutePath> find_route_path(const IStateView& state, const GID& from_signal_gid,
                                         const GID& to_signal_gid);

// Generate a stable route ID from the two signal GIDs and the tick.
GID make_route_id(const GID& from_signal_gid, const GID& to_signal_gid, uint64_t tick);

}  // namespace srk::common
