#pragma once

// libscenario_validation/include/scenario_validation/layer2_validator.hpp
//
// Layer 2: Cross-file semantic consistency checks.
//
// Requires both topology.json and objects.json, plus the station_sid from
// meta.json.  Detects:
//   L2-001  sID mismatch: object sID does not match meta.json station_sid.
//   L2-002  OT neighborID starting with "ZWR-" references unknown switch GID.
//   L2-003  OT neighborID starting with "BND-" references unknown boundary GID.
//   L2-004  ZWR trunk/straight/divergent neighborID references unknown OT GID.
//   L2-005  Signal governs_track_section references OT from different station.

#include "scenario_validation/validation_result.hpp"

#include <nlohmann/json.hpp>
#include <string>

namespace scenario_validation
{

class Layer2Validator
{
public:
    /// Validate cross-file consistency.
    /// @param station_sid  Value of "station_sid" from meta.json.
    /// @param topology     Contents of topology.json.
    /// @param objects      Contents of objects.json.
    [[nodiscard]] ValidationResult validate(const std::string& station_sid,
                                            const nlohmann::json& topology,
                                            const nlohmann::json& objects) const;
};

}  // namespace scenario_validation
