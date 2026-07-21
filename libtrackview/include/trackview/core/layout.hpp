#pragma once

#include <compare>
#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace trackview
{

struct InfrastructureId
{
    std::uint64_t value = 0;
    auto operator<=>(const InfrastructureId&) const = default;
};

struct Point
{
    double x = 0.0;
    double y = 0.0;
    auto operator<=>(const Point&) const = default;
};

using Path = std::vector<Point>;

struct Canvas
{
    double width = 0.0;
    double height = 0.0;
};

struct TrackSectionGeometry
{
    InfrastructureId topology_id;
    Path path;
};

struct SwitchPorts
{
    Point trunk;
    Point straight;
    Point divergent;
};

struct SwitchGeometry
{
    InfrastructureId topology_id;
    SwitchPorts ports;
};

enum class TrackSide
{
    A,
    B
};

enum class FacingDirection
{
    TowardsA,
    TowardsB
};

struct TrackAttachment
{
    InfrastructureId track_id;
    TrackSide reference_side = TrackSide::A;
    double offset = 0.0;
    double lateral = 0.0;
};

struct SignalGeometry
{
    InfrastructureId object_id;
    TrackAttachment attachment;
    FacingDirection facing = FacingDirection::TowardsB;
};

struct LabelGeometry
{
    std::string text;
    Point anchor;
};

// This variant is the closed vocabulary of a track schematic. Mechanical
// control panels are a separate bounded context, not extra track elements.
using TrackLayoutElement =
    std::variant<TrackSectionGeometry, SwitchGeometry, SignalGeometry>;

struct TrackLayout
{
    int schema_version = 1;
    std::string layout_id;
    std::string station_sid;
    std::string style_hint;
    Canvas canvas;
    std::vector<TrackLayoutElement> elements;
    std::vector<LabelGeometry> labels;
};

}  // namespace trackview

template<>
struct std::hash<trackview::InfrastructureId>
{
    std::size_t operator()(trackview::InfrastructureId id) const noexcept
    {
        return std::hash<std::uint64_t>{}(id.value);
    }
};
