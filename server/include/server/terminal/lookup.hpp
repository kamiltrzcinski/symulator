// server/include/server/terminal/lookup.hpp
//
// Argument-resolution helpers shared by terminal commands.  Every object
// argument may be given either as a decimal UID or as the object's pID.

#pragma once

#include "engine/core/engine_snapshot.hpp"
#include "engine/core/fleet_registry.hpp"
#include "engine/core/types.hpp"

#include <optional>
#include <string_view>

namespace server::terminal
{

/// Parse a full decimal unsigned integer.  Returns std::nullopt on any
/// non-numeric input (so pIDs starting with a digit still fall through).
std::optional<std::uint64_t> parse_uint(std::string_view text);

/// Resolve a consist by decimal UID or pID.
std::optional<engine::core::UID> resolve_consist_uid(const engine::core::FleetRegistry& fleet,
                                                     std::string_view arg);

/// Resolve a boundary node by decimal UID or pID against a world snapshot.
std::optional<engine::core::UID> resolve_boundary_uid(const engine::core::EngineSnapshot& snap,
                                                      std::string_view arg);

}  // namespace server::terminal
