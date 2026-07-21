#include <trackview/core/layout_validator.hpp>

#include <cmath>
#include <type_traits>
#include <unordered_set>

namespace trackview
{
namespace
{

bool is_finite(Point point)
{
    return std::isfinite(point.x) && std::isfinite(point.y);
}

void add(std::vector<LayoutDiagnostic>& diagnostics, std::string rule,
         std::optional<InfrastructureId> id, std::string message)
{
    diagnostics.push_back(
        {DiagnosticSeverity::Error, std::move(rule), id, std::move(message)});
}

}  // namespace

std::vector<LayoutDiagnostic>
LayoutValidator::validate(const IInfrastructureCatalog& catalog,
                          const TrackLayout& layout) const
{
    std::vector<LayoutDiagnostic> diagnostics;
    std::unordered_set<InfrastructureId> topology_geometries;
    std::unordered_set<InfrastructureId> track_geometries;
    std::unordered_set<InfrastructureId> object_geometries;

    for (const auto& element : layout.elements)
        if (const auto* track = std::get_if<TrackSectionGeometry>(&element))
            track_geometries.insert(track->topology_id);

    if (layout.schema_version != 1)
        add(diagnostics, "LAY-001", std::nullopt, "Unsupported layout schema version");
    if (layout.layout_id.empty() || layout.station_sid.empty())
        add(diagnostics, "LAY-002", std::nullopt,
            "Layout and station identifiers must not be empty");
    if (!(layout.canvas.width > 0.0) || !(layout.canvas.height > 0.0) ||
        !std::isfinite(layout.canvas.width) || !std::isfinite(layout.canvas.height))
        add(diagnostics, "LAY-003", std::nullopt,
            "Canvas dimensions must be finite and positive");

    for (const auto& element : layout.elements)
    {
        std::visit(
            [&](const auto& item) {
                using T = std::decay_t<decltype(item)>;
                if constexpr (std::is_same_v<T, TrackSectionGeometry>)
                {
                    if (!catalog.contains_track(item.topology_id))
                        add(diagnostics, "LAY-101", item.topology_id,
                            "Unknown track section identifier");
                    if (item.path.size() < 2)
                        add(diagnostics, "LAY-102", item.topology_id,
                            "Track path requires at least two points");
                    for (const auto point : item.path)
                        if (!is_finite(point))
                            add(diagnostics, "LAY-103", item.topology_id,
                                "Track path contains a non-finite coordinate");
                    if (!topology_geometries.insert(item.topology_id).second)
                        add(diagnostics, "LAY-104", item.topology_id,
                            "Duplicate topology geometry");
                }
                else if constexpr (std::is_same_v<T, SwitchGeometry>)
                {
                    if (!catalog.contains_switch(item.topology_id))
                        add(diagnostics, "LAY-201", item.topology_id,
                            "Unknown switch identifier");
                    if (!is_finite(item.ports.trunk) || !is_finite(item.ports.straight) ||
                        !is_finite(item.ports.divergent))
                        add(diagnostics, "LAY-202", item.topology_id,
                            "Switch contains a non-finite port coordinate");
                    if (!topology_geometries.insert(item.topology_id).second)
                        add(diagnostics, "LAY-104", item.topology_id,
                            "Duplicate topology geometry");
                }
                else if constexpr (std::is_same_v<T, SignalGeometry>)
                {
                    if (!catalog.contains_signal(item.object_id))
                        add(diagnostics, "LAY-301", item.object_id,
                            "Unknown signal identifier");
                    if (!catalog.contains_track(item.attachment.track_id))
                        add(diagnostics, "LAY-302", item.attachment.track_id,
                            "Signal is attached to an unknown track section");
                    else if (catalog.contains_signal(item.object_id) &&
                             !catalog.signal_governs_track(item.object_id,
                                                           item.attachment.track_id))
                        add(diagnostics, "LAY-303", item.object_id,
                            "Signal is attached to a track it does not govern");
                    if (!std::isfinite(item.attachment.offset) ||
                        item.attachment.offset < 0.0 || item.attachment.offset > 1.0)
                        add(diagnostics, "LAY-304", item.object_id,
                            "Signal attachment offset must be in range 0..1");
                    if (!std::isfinite(item.attachment.lateral))
                        add(diagnostics, "LAY-305", item.object_id,
                            "Signal lateral offset must be finite");
                    if (!object_geometries.insert(item.object_id).second)
                        add(diagnostics, "LAY-306", item.object_id,
                            "Duplicate signal geometry");
                    if (!track_geometries.contains(item.attachment.track_id))
                        add(diagnostics, "LAY-307", item.object_id,
                            "Signal attachment requires track geometry in this layout");
                }
            },
            element);
    }
    for (const auto& label : layout.labels)
        if (label.text.empty() || !is_finite(label.anchor))
            add(diagnostics, "LAY-401", std::nullopt,
                "Labels require non-empty text and finite coordinates");
    return diagnostics;
}

}  // namespace trackview
