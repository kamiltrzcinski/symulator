#include <srk/common/route_graph.hpp>

#include <algorithm>
#include <optional>
#include <queue>
#include <unordered_map>
#include <vector>

namespace srk::common
{

// ── Graph neighbour enumeration ───────────────────────────────────────────────
// Returns the UIDs reachable from `node` when arriving from `came_from`.
// Uses directional traversal so we never double back.
// BoundaryNode has no outgoing edges (returns empty list).

static std::vector<UID> topology_neighbors(const IStateView& state, UID node, UID came_from)
{
    if (const TrackSection* ts = state.find_track_section(node))
    {
        // If we came from sideA, go to sideB and vice versa.
        // On the start node (came_from is zero), return both sides.
        if (came_from.value == 0)
            return {ts->side_a.neighbor_uid, ts->side_b.neighbor_uid};
        if (ts->side_a.neighbor_uid == came_from)
            return {ts->side_b.neighbor_uid};
        if (ts->side_b.neighbor_uid == came_from)
            return {ts->side_a.neighbor_uid};
        return {};
    }

    if (const Switch* sw = state.find_switch(node))
    {
        // Three legs: trunk, straight, divergent.
        // Return the two legs that are NOT came_from.
        std::vector<UID> all = {sw->trunk.neighbor_uid, sw->straight.neighbor_uid,
                                sw->divergent.neighbor_uid};
        all.erase(std::remove(all.begin(), all.end(), came_from), all.end());
        return all;
    }

    // BoundaryNode: dead end.
    return {};
}

// Returns true if uid is a topology node we can traverse (section or switch).
// BoundaryNodes are valid targets but not traversable further.
static bool is_traversable_node(const IStateView& state, UID uid)
{
    return state.find_track_section(uid) != nullptr || state.find_switch(uid) != nullptr;
}

// ── BFS ───────────────────────────────────────────────────────────────────────
// Returns the path of UIDs from start to goal (inclusive), or nullopt.

static std::optional<std::vector<UID>> bfs(const IStateView& state, UID start, UID goal)
{
    // prev[node] = predecessor UID.  Use zero UID as "no predecessor" (start).
    std::unordered_map<UID, UID, std::hash<UID>> prev;
    prev.emplace(start, UID{});

    struct BfsNode
    {
        UID uid;
        UID came_from;
    };
    std::queue<BfsNode> q;
    q.push({start, UID{}});

    while (!q.empty())
    {
        auto [cur, from] = q.front();
        q.pop();

        if (cur == goal)
        {
            // Reconstruct path.
            std::vector<UID> path;
            UID n = goal;
            while (n.value != 0)
            {
                path.push_back(n);
                n = prev.at(n);
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (const UID& next : topology_neighbors(state, cur, from))
        {
            if (prev.count(next))
                continue;  // already visited
            prev.emplace(next, cur);
            if (is_traversable_node(state, next))
                q.push({next, cur});
            // BoundaryNode: add to prev (so goal can be found), but don't enqueue.
        }
    }
    return std::nullopt;
}

// ── Switch position determination ────────────────────────────────────────────

static SwitchPosition required_switch_position(const Switch& sw, UID came_from, UID going_to)
{
    // Approaching from trunk: position determines which leg to use.
    if (sw.trunk.neighbor_uid == came_from)
    {
        if (sw.straight.neighbor_uid == going_to)
            return SwitchPosition::STRAIGHT;
        if (sw.divergent.neighbor_uid == going_to)
            return SwitchPosition::DIVERGENT;
    }
    // Approaching from straight or divergent tip (traversing against the point).
    if (sw.straight.neighbor_uid == came_from)
        return SwitchPosition::STRAIGHT;
    if (sw.divergent.neighbor_uid == came_from)
        return SwitchPosition::DIVERGENT;
    return SwitchPosition::STRAIGHT;
}

// ── Public API ────────────────────────────────────────────────────────────────

std::optional<RoutePath> find_route_path(const IStateView& state, UID from_signal_uid,
                                         UID to_signal_uid)
{
    const Signal* entry = state.find_signal(from_signal_uid);
    const Signal* exit = state.find_signal(to_signal_uid);
    if (!entry || !exit)
        return std::nullopt;

    const UID start = entry->governs_section_uid;
    const UID goal = exit->governs_section_uid;

    if (start == goal)
    {
        // Same section — zero-length route.  Unusual but allowed for shunting signals.
        RoutePath rp;
        rp.from_signal_uid = from_signal_uid;
        rp.to_signal_uid = to_signal_uid;
        rp.nodes.push_back({RoutePathNode::Kind::TRACK_SECTION, start});
        rp.section_uids.push_back(start);
        // Find derailers guarding this section.
        state.for_each_derailer(
            [&](const Derailer& d)
            {
                if (d.guards_section_uid == start)
                    rp.derailer_uids.push_back(d.uid);
            });
        return rp;
    }

    auto path_uids = bfs(state, start, goal);
    if (!path_uids)
        return std::nullopt;

    RoutePath rp;
    rp.from_signal_uid = from_signal_uid;
    rp.to_signal_uid = to_signal_uid;

    for (std::size_t i = 0; i < path_uids->size(); ++i)
    {
        const UID uid = (*path_uids)[i];

        if (state.find_track_section(uid))
        {
            rp.nodes.push_back({RoutePathNode::Kind::TRACK_SECTION, uid});
            rp.section_uids.push_back(uid);
        }
        else if (const Switch* sw = state.find_switch(uid))
        {
            const UID from = (i > 0) ? (*path_uids)[i - 1] : UID{};
            const UID to = (i + 1 < path_uids->size()) ? (*path_uids)[i + 1] : UID{};
            SwitchPosition pos = required_switch_position(*sw, from, to);
            rp.nodes.push_back({RoutePathNode::Kind::SWITCH, uid, pos});
            rp.switch_uids.push_back(uid);
        }
    }

    // Collect derailers guarding any section on the path.
    const auto& sections = rp.section_uids;
    state.for_each_derailer(
        [&](const Derailer& d)
        {
            if (std::find(sections.begin(), sections.end(), d.guards_section_uid) != sections.end())
            {
                rp.derailer_uids.push_back(d.uid);
            }
        });

    return rp;
}

UID make_route_uid(UID from_signal_uid, UID to_signal_uid, uint64_t tick)
{
    // Synthesize a route UID in the OPERATIONS/ROUTE domain.
    // Use lower 16 bits of from_signal + to_signal XOR + tick modulo for uniqueness.
    const uint16_t scope =
        static_cast<uint16_t>(uid_scope(from_signal_uid) ^ uid_scope(to_signal_uid));
    const uint16_t instance =
        static_cast<uint16_t>((uid_instance(from_signal_uid) ^ uid_instance(to_signal_uid) ^
                               static_cast<uint16_t>(tick)) &
                              0xFFFFU);
    return make_uid(UIDDomain::OPERATIONS, UIDKind::ROUTE, scope, instance);
}

}  // namespace srk::common
