// engine/src/spawn_resolver.cpp

#include "engine/core/spawn_resolver.hpp"

#include "engine/core/track_model.hpp"

namespace engine::core
{

const char* to_string(SpawnError e)
{
    switch (e)
    {
        case SpawnError::UNKNOWN_BOUNDARY:
            return "boundary node not found";
        case SpawnError::NO_ADJACENT_SECTION:
            return "no track section adjacent to the boundary node";
        case SpawnError::AMBIGUOUS:
            return "more than one track section references the boundary node";
        case SpawnError::SECTION_OCCUPIED:
            return "the section adjacent to the boundary node is not free";
    }
    return "unknown spawn error";
}

std::variant<SpawnPoint, SpawnError> resolve_spawn_at_boundary(const IStateView& state,
                                                               UID boundary_uid)
{
    if (!state.find_boundary_node(boundary_uid))
        return SpawnError::UNKNOWN_BOUNDARY;

    const TrackSection* adjacent = nullptr;
    int matches = 0;
    state.for_each_track_section(
        [&](const TrackSection& ts)
        {
            if (ts.side_a.neighbor_uid == boundary_uid || ts.side_b.neighbor_uid == boundary_uid)
            {
                adjacent = &ts;
                ++matches;
            }
        });

    if (matches == 0)
        return SpawnError::NO_ADJACENT_SECTION;
    if (matches > 1)
        return SpawnError::AMBIGUOUS;
    if (adjacent->occupancy != TrackOccupancy::FREE)
        return SpawnError::SECTION_OCCUPIED;

    return SpawnPoint{.section_uid = adjacent->uid, .from_uid = boundary_uid};
}

}  // namespace engine::core
