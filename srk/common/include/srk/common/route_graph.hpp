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
    UID uid;
    // Only meaningful when kind == SWITCH.
    SwitchPosition required_position = SwitchPosition::STRAIGHT;
};

struct RoutePath
{
    UID from_signal_uid;
    UID to_signal_uid;
    std::vector<RoutePathNode> nodes;
    std::vector<UID> section_uids;   // pre-extracted for convenience
    std::vector<UID> switch_uids;    // pre-extracted for convenience
    std::vector<UID> derailer_uids;  // derailers guarding sections on the path
    std::vector<RoutePathNode> flank_switches; // switches required for flank protection
};

// Interface for injecting extra path validation and gathering logic (e.g., Flank Protection)
struct IRoutePathPolicy
{
    virtual ~IRoutePathPolicy() = default;
    // Called after the core BFS path is resolved. Can append additional required locks
    // to the path, or return false to indicate the path is invalid based on this policy.
    virtual bool apply(const IStateView& state, RoutePath& path) const = 0;
};

// Find a route path from the entry signal to the exit signal.
// Returns nullopt if no path exists in the topology, or if a policy rejects the path.
std::optional<RoutePath> find_route_path(const IStateView& state, UID from_signal_uid,
                                         UID to_signal_uid,
                                         const std::vector<const IRoutePathPolicy*>& policies = {});

// Generate a stable route UID from the two signal UIDs and the tick.
UID make_route_uid(UID from_signal_uid, UID to_signal_uid, uint64_t tick);

}  // namespace srk::common
