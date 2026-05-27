#pragma once

// libscenario_validation/include/scenario_validation/validation_result.hpp
//
// Shared result types for all validation layers.

#include <optional>
#include <string>
#include <vector>

namespace scenario_validation
{

struct ValidationIssue
{
    enum class Severity
    {
        WARNING,
        ERROR,
    };

    Severity severity;
    std::string code;     ///< Issue code, e.g. "L1-001" — stable for machine consumption.
    std::string message;  ///< Human-readable description.
    std::optional<std::string> gid;  ///< Affected object GID, if applicable.
};

struct ValidationResult
{
    std::vector<ValidationIssue> issues;

    /// True when there are no ERROR-severity issues.
    [[nodiscard]] bool ok() const noexcept;

    /// Number of ERROR-severity issues.
    [[nodiscard]] int error_count() const noexcept;

    /// Append all issues from another result.
    void merge(ValidationResult other);
};

}  // namespace scenario_validation
