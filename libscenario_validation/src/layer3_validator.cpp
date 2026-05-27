// libscenario_validation/src/layer3_validator.cpp

#include "scenario_validation/layer3_validator.hpp"

namespace scenario_validation
{

ValidationResult Layer3Validator::validate(const nlohmann::json& /*topology*/,
                                           const nlohmann::json& /*objects*/) const
{
    ValidationResult result;
    result.issues.push_back({ValidationIssue::Severity::WARNING, "L3-000",
                             "Layer 3 (route completeness + signal coverage) is not yet "
                             "implemented",
                             std::nullopt});
    return result;
}

}  // namespace scenario_validation
