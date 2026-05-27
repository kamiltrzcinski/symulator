// libscenario_validation/src/validation_result.cpp

#include "scenario_validation/validation_result.hpp"

#include <algorithm>

namespace scenario_validation
{

bool ValidationResult::ok() const noexcept
{
    return std::none_of(issues.begin(), issues.end(), [](const ValidationIssue& i)
                        { return i.severity == ValidationIssue::Severity::ERROR; });
}

int ValidationResult::error_count() const noexcept
{
    return static_cast<int>(
        std::count_if(issues.begin(), issues.end(), [](const ValidationIssue& i)
                      { return i.severity == ValidationIssue::Severity::ERROR; }));
}

void ValidationResult::merge(ValidationResult other)
{
    issues.insert(issues.end(), std::make_move_iterator(other.issues.begin()),
                  std::make_move_iterator(other.issues.end()));
}

}  // namespace scenario_validation
