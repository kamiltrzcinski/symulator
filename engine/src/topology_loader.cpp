// engine/src/topology_loader.cpp

#include "engine/core/topology_loader.hpp"
#include "engine/core/track_model.hpp"
#include "engine/core/types.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace engine::core
{

namespace
{

using json = nlohmann::json;

// ── Helpers ──────────────────────────────────────────────────────────────────

json read_json(const std::filesystem::path& path)
{
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("topology_loader: cannot open '" + path.string() + "'");
    json j;
    try
    {
        f >> j;
    }
    catch (const json::parse_error& e)
    {
        throw std::runtime_error("topology_loader: JSON parse error in '" + path.string() +
                                 "': " + e.what());
    }
    return j;
}

TrackPort parse_track_port(const json& j)
{
    TrackPort port;
    port.neighbor_gid = GID{j.at("neighborID").get<std::string>()};

    if (j.contains("itID"))
    {
        port.counter_gid = GID{j.at("itID").get<std::string>()};
        port.counter_kind = TrackPort::CounterKind::IT;
    }
    else if (j.contains("izID"))
    {
        port.counter_gid = GID{j.at("izID").get<std::string>()};
        port.counter_kind = TrackPort::CounterKind::IZ;
    }

    if (j.contains("signals"))
    {
        for (const auto& s : j.at("signals"))
            port.signal_gids.push_back(GID{s.get<std::string>()});
    }
    return port;
}

SwitchLeg parse_switch_leg(const json& j)
{
    SwitchLeg leg;
    leg.neighbor_gid = GID{j.at("neighborID").get<std::string>()};
    if (j.contains("izID"))
        leg.iz_gid = GID{j.at("izID").get<std::string>()};
    if (j.contains("signals"))
    {
        for (const auto& s : j.at("signals"))
            leg.signal_gids.push_back(GID{s.get<std::string>()});
    }
    return leg;
}

Signal::Type parse_signal_type(const std::string& s)
{
    if (s == "ENTRY")
        return Signal::Type::ENTRY;
    if (s == "DEPARTURE")
        return Signal::Type::DEPARTURE;
    if (s == "BLOCK")
        return Signal::Type::BLOCK;
    if (s == "SHUNTING")
        return Signal::Type::SHUNTING;
    throw std::runtime_error("topology_loader: unknown signal type '" + s + "'");
}

SignalAspect parse_initial_aspect(const std::string& s)
{
    if (s == "STOP" || s == "S1_STOP")
        return SignalAspect::S1_STOP;
    if (s == "S2_PROCEED")
        return SignalAspect::S2_PROCEED;
    if (s == "S3_PROCEED_40")
        return SignalAspect::S3_PROCEED_40;
    if (s == "MS1_STOP")
        return SignalAspect::MS1_STOP;
    if (s == "MS2_SHUNTING_ALLOWED")
        return SignalAspect::MS2_SHUNTING_ALLOWED;
    throw std::runtime_error("topology_loader: unknown initial_aspect '" + s + "'");
}

}  // anonymous namespace

// ── Public API ────────────────────────────────────────────────────────────────

ScenarioMeta load_scenario(EngineState& state, const std::filesystem::path& dir)
{
    // ── meta.json ────────────────────────────────────────────────────────────
    const json meta = read_json(dir / "meta.json");

    ScenarioMeta result;
    result.station_sid = meta.at("station_sid").get<std::string>();
    result.control_system_id = meta.at("control_system").get<std::string>();
    result.schema_version = meta.value("schema_version", 1);
    state.set_session_id(result.station_sid);

    // ── topology.json ────────────────────────────────────────────────────────
    const json topo = read_json(dir / "topology.json");

    for (const auto& j : topo.value("boundary_nodes", json::array()))
    {
        BoundaryNode bn;
        bn.gid = GID{j.at("gID").get<std::string>()};
        bn.pid = j.at("pID").get<std::string>();
        bn.sid = SID{j.at("sID").get<std::string>()};
        bn.description = j.value("description", "");
        state.insert_boundary_node(bn);
    }

    for (const auto& j : topo.value("track_sections", json::array()))
    {
        TrackSection ts;
        ts.gid = GID{j.at("gID").get<std::string>()};
        ts.pid = j.at("pID").get<std::string>();
        ts.sid = SID{j.at("sID").get<std::string>()};
        ts.side_a = parse_track_port(j.at("sideA"));
        ts.side_b = parse_track_port(j.at("sideB"));
        ts.length_m = j.value("lengthM", 0.0f);
        ts.electrified = j.value("electrified", false);
        ts.max_speed_kmh = j.value("maxSpeedKmh", 0);
        ts.occupancy = j.value("occupied", false) ? TrackOccupancy::OCCUPIED : TrackOccupancy::FREE;
        state.insert_track_section(ts);
    }

    for (const auto& j : topo.value("switches", json::array()))
    {
        Switch sw;
        sw.gid = GID{j.at("gID").get<std::string>()};
        sw.pid = j.at("pID").get<std::string>();
        sw.sid = SID{j.at("sID").get<std::string>()};
        sw.type_id = j.value("typeID", "");
        sw.trunk = parse_switch_leg(j.at("trunk"));
        sw.straight = parse_switch_leg(j.at("straight"));
        sw.divergent = parse_switch_leg(j.at("divergent"));
        sw.length_m = j.value("lengthM", 0.0f);
        sw.max_speed_straight_kmh = j.value("maxSpeedStraightKmh", 0);
        sw.max_speed_divergent_kmh = j.value("maxSpeedDivergentKmh", 0);
        state.insert_switch(sw);
    }

    // ── objects.json (optional) ──────────────────────────────────────────────
    const auto objects_path = dir / "objects.json";
    if (std::filesystem::exists(objects_path))
    {
        const json objs = read_json(objects_path);

        for (const auto& j : objs.value("signals", json::array()))
        {
            Signal sig;
            sig.gid = GID{j.at("gID").get<std::string>()};
            sig.pid = j.at("pID").get<std::string>();
            sig.sid = SID{j.at("sID").get<std::string>()};
            sig.type_id = j.value("typeID", "");
            sig.type = parse_signal_type(j.at("type").get<std::string>());
            sig.governs_track_section_gid = GID{j.at("governs_track_section").get<std::string>()};
            sig.current_aspect = parse_initial_aspect(j.value("initial_aspect", "STOP"));
            state.insert_signal(sig);
        }

        for (const auto& j : objs.value("derailers", json::array()))
        {
            Derailer der;
            der.gid = GID{j.at("gID").get<std::string>()};
            der.pid = j.at("pID").get<std::string>();
            der.sid = SID{j.at("sID").get<std::string>()};
            der.type_id = j.value("typeID", "");
            der.guards_track_section_gid = GID{j.at("guards_track_section").get<std::string>()};
            der.state = DerailerState::LOCKED;
            state.insert_derailer(der);
        }
    }

    return result;
}

}  // namespace engine::core
