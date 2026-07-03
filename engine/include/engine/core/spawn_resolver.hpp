// engine/include/engine/core/spawn_resolver.hpp
// Pure domain helper: resolve the spawn point behind a boundary node.
//
// BoundaryNode carries no back-reference to its adjacent section, so the
// resolver scans track sections for the unique one whose port references the
// boundary.  Used by the server terminal's spawn command; TrainScheduler (E5)
// will reuse it for timetable-driven spawning.

#pragma once

#include "engine/core/state_view.hpp"
#include "engine/core/types.hpp"

#include <variant>

namespace engine::core
{

struct SpawnPoint
{
    UID section_uid;  ///< Initial section for the spawned train.
    UID from_uid;     ///< The boundary node — determines which direction is "ahead".
};

enum class SpawnError
{
    UNKNOWN_BOUNDARY,     ///< No BoundaryNode with this UID.
    NO_ADJACENT_SECTION,  ///< No track section references the boundary.
    AMBIGUOUS,            ///< More than one section references the boundary.
    SECTION_OCCUPIED,     ///< The adjacent section is not FREE.
};

const char* to_string(SpawnError e);

std::variant<SpawnPoint, SpawnError> resolve_spawn_at_boundary(const IStateView& state,
                                                               UID boundary_uid);

}  // namespace engine::core
