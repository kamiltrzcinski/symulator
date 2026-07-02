// engine/include/engine/core/topology_loader.hpp
// Loads a scenario directory (meta.json + topology.json + objects.json)
// into a mutable EngineState.
//
// Throws std::runtime_error on missing files, malformed JSON, or unknown values.

#pragma once

#include "engine/core/engine_state.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace engine::core
{

struct ScenarioMeta
{
    std::string station_sid;        // from meta.json "station_sid"
    std::string control_system_id;  // from meta.json "control_system"
    int schema_version = 0;         // from meta.json "schema_version"
};

/// Load a complete scenario directory into `state`.
///
/// Reads (in order):
///   1. `<dir>/meta.json`     — session metadata
///   2. `<dir>/topology.json` — boundary nodes, track sections, switches
///   3. `<dir>/objects.json`  — signals, derailers (optional, skipped if absent)
///
/// Returns ScenarioMeta with the parsed control_system_id the caller should use
/// to look up the IControlSystem in ControlSystemRegistry.
///
/// @throws std::runtime_error on any IO or parse error.
ScenarioMeta load_scenario(EngineState& state, const std::filesystem::path& scenario_dir);

/// Load multiple scenario directories into the same `state`, forming one
/// multi-station world. Calls `load_scenario()` for each directory in order;
/// `insert_*()` on EngineState is idempotent, so UID collisions across
/// scenarios are safe no-ops rather than overwrites.
///
/// The first directory in `scenario_dirs` is the "primary" scenario: its
/// session id wins even though every `load_scenario()` call sets it, since
/// each station's own `station_sid` would otherwise clobber the previous one.
///
/// @throws std::runtime_error on any IO or parse error, or if `scenario_dirs` is empty.
std::vector<ScenarioMeta> load_world(EngineState& state,
                                     const std::vector<std::filesystem::path>& scenario_dirs);

}  // namespace engine::core
