#pragma once

#include "infrastructure_catalog.hpp"

#include <optional>
#include <string>
#include <vector>

namespace trackview
{

enum class DiagnosticSeverity
{
    Error,
    Warning
};

struct LayoutDiagnostic
{
    DiagnosticSeverity severity = DiagnosticSeverity::Error;
    std::string rule_id;
    std::optional<InfrastructureId> infrastructure_id;
    std::string message;
};

class ILayoutValidator
{
public:
    virtual ~ILayoutValidator() = default;
    [[nodiscard]] virtual std::vector<LayoutDiagnostic>
    validate(const IInfrastructureCatalog& catalog, const TrackLayout& layout) const = 0;
};

class LayoutValidator final : public ILayoutValidator
{
public:
    [[nodiscard]] std::vector<LayoutDiagnostic>
    validate(const IInfrastructureCatalog& catalog,
             const TrackLayout& layout) const override;
};

}  // namespace trackview
