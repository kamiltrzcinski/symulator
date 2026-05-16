#include <srk/common/route_graph.hpp>

#include <algorithm>
#include <format>
#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>

namespace srk::common
{

// ── Graph neighbour enumeration ───────────────────────────────────────────────
// Returns the GIDs reachable from `node` when arriving from `came_from`.
// Uses directional traversal so we never double back.
// BoundaryNode has no outgoing edges (returns empty list).

static std::vector<GID> topology_neighbors(const IStateView& state, const GID& node,
                                           const GID& came_from)
{
    if (const TrackSection* ts = state.find_track_section(node))
    {
        // If we came from sideA, go to sideB and vice versa.
        // On the start node (came_from is empty), return both sides.
        if (came_from.value.empty())
            return {ts->side_a.neighbor_gid, ts->side_b.neighbor_gid};
        if (ts->side_a.neighbor_gid == came_from)
            return {ts->side_b.neighbor_gid};
        if (ts->side_b.neighbor_gid == came_from)
            return {ts->side_a.neighbor_gid};
        return {};
    }

    if (const Switch* sw = state.find_switch(node))
    {
        // Three legs: trunk, straight, divergent.
        // Return the two legs that are NOT came_from.
        std::vector<GID> all = {sw->trunk.neighbor_gid, sw->straight.neighbor_gid,
                                sw->divergent.neighbor_gid};
        all.erase(std::remove(all.begin(), all.end(), came_from), all.end());
        return all;
    }

    // BoundaryNode: dead end.
    return {};
}

// Returns true if gid is a topology node we can traverse (section or switch).
// BoundaryNodes are valid targets but not traversable further.
static bool is_traversable_node(const IStateView& state, const GID& gid)
{
    return state.find_track_section(gid) != nullptr || state.find_switch(gid) != nullptr;
}

// ── BFS ───────────────────────────────────────────────────────────────────────
// Returns the path of GIDs from start to goal (inclusive), or nullopt.

static std::optional<std::vector<GID>> bfs(const IStateView& state, const GID& start,
                                           const GID& goal)
{
    // prev[node] = predecessor GID.  Use empty GID as "no predecessor" (start).
    std::unordered_map<GID, GID, std::hash<GID>> prev;
    prev.emplace(start, GID{});

    struct BfsNode
    {
        GID gid;
        GID came_from;
    };
    std::queue<BfsNode> q;
    q.push({start, GID{}});

    while (!q.empty())
    {
        auto [cur, from] = q.front();
        q.pop();

        if (cur == goal)
        {
            // Reconstruct path.
            std::vector<GID> path;
            GID n = goal;
            while (!n.value.empty())
            {
                path.push_back(n);
                n = prev.at(n);
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (const GID& next : topology_neighbors(state, cur, from))
        {
            if (prev.count(next))
                continue;  // already visited
            prev.emplace(next, cur);
            if (is_traversable_node(state, next))
                q.push({next, cur});
            // BoundaryNode: add to prev (so goal can be found), but don't enqueue.
            // Allow finding the goal even if it's a BoundaryNode (unlikely for signals).
        }
    }
    return std::nullopt;
}

// ── Switch position determination ────────────────────────────────────────────
// Given a switch gid, the previous path node (came_from), and the next path
// node (going_to), determine which SwitchPosition is required.

static SwitchPosition required_switch_position(const Switch& sw, const GID& came_from,
                                               const GID& going_to)
{
    // Approaching from trunk: position determines which leg to use.
    if (sw.trunk.neighbor_gid == came_from)
    {
        if (sw.straight.neighbor_gid == going_to)
            return SwitchPosition::STRAIGHT;
        if (sw.divergent.neighbor_gid == going_to)
            return SwitchPosition::DIVERGENT;
    }
    // Approaching from straight or divergent tip (traversing against the point).
    // Position must match the leg we entered from so the switch is confirmed.
    if (sw.straight.neighbor_gid == came_from)
        return SwitchPosition::STRAIGHT;
    if (sw.divergent.neighbor_gid == came_from)
        return SwitchPosition::DIVERGENT;
    return SwitchPosition::STRAIGHT;
}

// ── Public API ────────────────────────────────────────────────────────────────

std::optional<RoutePath> find_route_path(const IStateView& state, const GID& from_signal_gid,
                                         const GID& to_signal_gid)
{
    const Signal* entry = state.find_signal(from_signal_gid);
    const Signal* exit = state.find_signal(to_signal_gid);
    if (!entry || !exit)
        return std::nullopt;

    const GID& start = entry->governs_track_section_gid;
    const GID& goal = exit->governs_track_section_gid;

    if (start == goal)
    {
        // Same section — zero-length route.  Unusual but allowed for shunting signals.
        RoutePath rp;
        rp.from_signal_gid = from_signal_gid;
        rp.to_signal_gid = to_signal_gid;
        rp.nodes.push_back({RoutePathNode::Kind::TRACK_SECTION, start});
        rp.section_gids.push_back(start);
        // Find derailers guarding this section.
        state.for_each_derailer(
            [&](const Derailer& d)
            {
                if (d.guards_track_section_gid == start)
                    rp.derailer_gids.push_back(d.gid);
            });
        return rp;
    }

    auto path_gids = bfs(state, start, goal);
    if (!path_gids)
        return std::nullopt;

    RoutePath rp;
    rp.from_signal_gid = from_signal_gid;
    rp.to_signal_gid = to_signal_gid;

    for (std::size_t i = 0; i < path_gids->size(); ++i)
    {
        const GID& gid = (*path_gids)[i];

        if (state.find_track_section(gid))
        {
            rp.nodes.push_back({RoutePathNode::Kind::TRACK_SECTION, gid});
            rp.section_gids.push_back(gid);
        }
        else if (const Switch* sw = state.find_switch(gid))
        {
            const GID& from = (i > 0) ? (*path_gids)[i - 1] : GID{};
            const GID& to = (i + 1 < path_gids->size()) ? (*path_gids)[i + 1] : GID{};
            SwitchPosition pos = required_switch_position(*sw, from, to);
            rp.nodes.push_back({RoutePathNode::Kind::SWITCH, gid, pos});
            rp.switch_gids.push_back(gid);
        }
    }

    // Collect derailers guarding any section on the path.
    const auto& sections = rp.section_gids;
    state.for_each_derailer(
        [&](const Derailer& d)
        {
            if (std::find(sections.begin(), sections.end(), d.guards_track_section_gid) !=
                sections.end())
            {
                rp.derailer_gids.push_back(d.gid);
            }
        });

    return rp;
}

GID make_route_id(const GID& from_signal_gid, const GID& to_signal_gid, uint64_t tick)
{
    return GID{std::format("RTE-{}-{}-{}", from_signal_gid.value, to_signal_gid.value, tick)};
}

}  // namespace srk::common
