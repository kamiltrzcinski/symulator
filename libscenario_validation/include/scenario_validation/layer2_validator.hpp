#pragma once

// libscenario_validation/include/scenario_validation/layer2_validator.hpp
//
// Layer 2: Cross-file semantic consistency checks (UID-based format).
//
// Requires topology.json and objects.json, plus the station_instance from
// meta.json (numeric station instance, e.g. 1 for GOr).  Detects:
//   L2-001  SCOPE mismatch: object UID has a different station SCOPE.
//   L2-002  Track section neighborUID references unknown switch UID.
//   L2-003  Track section neighborUID references unknown boundary_node UID.
//   L2-004  Switch leg neighborUID references unknown track_section UID.
//   L2-005  Signal governs_section references unknown track_section UID.

#include "scenario_validation/validation_result.hpp"

#include <cstdint>
#include <nlohmann/json.hpp>

namespace scenario_validation
{

class Layer2Validator
{
public:
    /// Validate cross-file consistency.
    /// @param station_instance  Numeric station instance (SCOPE) from meta.json/stations.json.
    /// @param topology          Contents of topology.json.
    /// @param objects           Contents of objects.json.
    [[nodiscard]] ValidationResult validate(int station_instance, const nlohmann::json& topology,
                                            const nlohmann::json& objects) const;
};

}  // namespace scenario_validation
