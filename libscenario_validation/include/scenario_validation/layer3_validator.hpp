#pragma once

// libscenario_validation/include/scenario_validation/layer3_validator.hpp
//
// Layer 3: Route completeness and signal coverage (interface stub).
//
// This layer is defined but not yet implemented.
// validate() always returns a single WARNING "L3-000 not implemented" issue.

#include "scenario_validation/validation_result.hpp"

#include <nlohmann/json.hpp>

namespace scenario_validation
{

class Layer3Validator
{
public:
    /// Route completeness and signal-coverage validation.
    /// @note Not yet implemented — returns a NOT_IMPLEMENTED warning.
    [[nodiscard]] ValidationResult validate(const nlohmann::json& topology,
                                            const nlohmann::json& objects) const;
};

}  // namespace scenario_validation
