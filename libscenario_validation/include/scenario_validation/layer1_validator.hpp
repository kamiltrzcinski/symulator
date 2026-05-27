#pragma once

// libscenario_validation/include/scenario_validation/layer1_validator.hpp
//
// Layer 1: JSON-level syntax and schema checks.
//
// Validates topology.json and objects.json in isolation (no cross-file
// consistency).  Detects:
//   L1-001  Missing required field (gID, pID, or sID absent / empty).
//   L1-002  Duplicate GID detected within a file.
//   L1-003  izID referenced in an OT edge has no match in any ZWR Iz set.
//   L1-004  Signal listed in an OT edge.signals[] not present in objects.json.
//   L1-005  Signal governs_track_section references unknown OT GID.
//   L1-006  Invalid initial_aspect value for a signal.

#include "scenario_validation/validation_result.hpp"

#include <nlohmann/json.hpp>

namespace scenario_validation
{

class Layer1Validator
{
public:
    /// Validate raw parsed JSON files.
    /// @param topology  Contents of topology.json.
    /// @param objects   Contents of objects.json.
    [[nodiscard]] ValidationResult validate(const nlohmann::json& topology,
                                            const nlohmann::json& objects) const;
};

}  // namespace scenario_validation
