#include <trackview/core/layout_reader.hpp>

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

namespace trackview
{
namespace
{

using Json = nlohmann::json;

Point read_point(const Json& value)
{
    return {value.at("x").get<double>(), value.at("y").get<double>()};
}

InfrastructureId read_id(const Json& value, const char* field)
{
    return {value.at(field).get<std::uint64_t>()};
}

TrackSide read_side(const std::string& side)
{
    if (side == "A")
        return TrackSide::A;
    if (side == "B")
        return TrackSide::B;
    throw std::runtime_error("layout_reader: signal side must be A or B");
}

FacingDirection read_facing(const std::string& facing)
{
    if (facing == "towards_A")
        return FacingDirection::TowardsA;
    if (facing == "towards_B")
        return FacingDirection::TowardsB;
    throw std::runtime_error(
        "layout_reader: signal facing must be towards_A or towards_B");
}

}  // namespace

TrackLayout JsonLayoutReader::read(const std::filesystem::path& path) const
{
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("layout_reader: cannot open '" + path.string() + "'");

    Json root;
    try
    {
        input >> root;
        TrackLayout result;
        result.schema_version = root.at("schema_version").get<int>();
        if (result.schema_version != 1)
            throw std::runtime_error("layout_reader: unsupported schema_version " +
                                     std::to_string(result.schema_version));
        if (root.at("kind").get<std::string>() != "track_schematic")
            throw std::runtime_error("layout_reader: kind must be track_schematic");

        result.layout_id = root.at("layout_id").get<std::string>();
        result.station_sid = root.at("station_sid").get<std::string>();
        result.style_hint = root.value("style_hint", "ebiscreen");
        const auto& canvas = root.at("canvas");
        result.canvas = {canvas.at("width").get<double>(),
                         canvas.at("height").get<double>()};

        for (const auto& element : root.at("elements"))
        {
            const auto kind = element.at("kind").get<std::string>();
            if (kind == "track_section")
            {
                TrackSectionGeometry geometry{read_id(element, "topology_uid"), {}};
                for (const auto& point : element.at("path"))
                    geometry.path.push_back(read_point(point));
                result.elements.emplace_back(std::move(geometry));
            }
            else if (kind == "switch")
            {
                const auto& ports = element.at("ports");
                result.elements.emplace_back(SwitchGeometry{
                    read_id(element, "topology_uid"),
                    {read_point(ports.at("trunk")), read_point(ports.at("straight")),
                     read_point(ports.at("divergent"))}});
            }
            else if (kind == "signal")
            {
                const auto& attachment = element.at("attachment");
                result.elements.emplace_back(SignalGeometry{
                    read_id(element, "object_uid"),
                    {read_id(attachment, "topology_uid"),
                     read_side(attachment.at("side").get<std::string>()),
                     attachment.at("offset").get<double>(),
                     attachment.value("lateral", 0.0)},
                    read_facing(element.at("facing").get<std::string>())});
            }
            else
            {
                throw std::runtime_error("layout_reader: unsupported element kind '" + kind +
                                         "'");
            }
        }

        for (const auto& label : root.value("labels", Json::array()))
            result.labels.push_back(
                {label.at("text").get<std::string>(), read_point(label.at("anchor"))});
        return result;
    }
    catch (const nlohmann::json::exception& error)
    {
        throw std::runtime_error("layout_reader: invalid layout in '" + path.string() +
                                 "': " + error.what());
    }
}

}  // namespace trackview
