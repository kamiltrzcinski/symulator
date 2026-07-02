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

UID require_uid_field(const json& j, const char* key, const std::filesystem::path& path)
{
    if (!j.contains(key) || !j.at(key).is_number_unsigned())
        throw std::runtime_error(std::string("topology_loader: missing or invalid UID field '") +
                                 key + "' in " + path.string());
    return UID{j.at(key).get<std::uint64_t>()};
}

// Derive the station UID from any infrastructure object's own UID.
// The SCOPE field of any INFRASTRUCTURE uid IS the station instance number.
UID station_uid_from_object_uid(UID obj_uid)
{
    return make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::STATION, uid_scope(obj_uid), 1);
}

TrackPort parse_track_port(const json& j, const std::filesystem::path& path)
{
    TrackPort port;
    port.neighbor_uid = require_uid_field(j, "neighborUID", path);

    if (j.contains("itUID"))
    {
        port.counter_uid = require_uid_field(j, "itUID", path);
        port.counter_kind = TrackPort::CounterKind::IT;
    }
    else if (j.contains("izUID"))
    {
        port.counter_uid = require_uid_field(j, "izUID", path);
        port.counter_kind = TrackPort::CounterKind::IZ;
    }

    if (j.contains("signalUIDs"))
    {
        for (const auto& s : j.at("signalUIDs"))
            port.signal_uids.push_back(UID{s.get<std::uint64_t>()});
    }
    return port;
}

SwitchLeg parse_switch_leg(const json& j, const std::filesystem::path& path)
{
    SwitchLeg leg;
    leg.neighbor_uid = require_uid_field(j, "neighborUID", path);
    if (j.contains("izUID"))
        leg.iz_uid = require_uid_field(j, "izUID", path);
    if (j.contains("signalUIDs"))
    {
        for (const auto& s : j.at("signalUIDs"))
            leg.signal_uids.push_back(UID{s.get<std::uint64_t>()});
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
    const auto topo_path = dir / "topology.json";

    for (const auto& j : topo.value("boundary_nodes", json::array()))
    {
        BoundaryNode bn;
        bn.uid = require_uid_field(j, "uid", topo_path);
        bn.pid = j.at("pID").get<std::string>();
        bn.station_uid = station_uid_from_object_uid(bn.uid);
        bn.description = j.value("description", "");
        state.insert_boundary_node(bn);
    }

    for (const auto& j : topo.value("track_sections", json::array()))
    {
        TrackSection ts;
        ts.uid = require_uid_field(j, "uid", topo_path);
        ts.pid = j.at("pID").get<std::string>();
        ts.station_uid = station_uid_from_object_uid(ts.uid);
        ts.side_a = parse_track_port(j.at("sideA"), topo_path);
        ts.side_b = parse_track_port(j.at("sideB"), topo_path);
        ts.length_m = j.value("lengthM", 0.0f);
        ts.electrified = j.value("electrified", false);
        ts.max_speed_kmh = j.value("maxSpeedKmh", 0);
        ts.occupancy = j.value("occupied", false) ? TrackOccupancy::OCCUPIED : TrackOccupancy::FREE;
        ts.station_section = j.value("station_section", true);
        state.insert_track_section(ts);
    }

    for (const auto& j : topo.value("switches", json::array()))
    {
        Switch sw;
        sw.uid = require_uid_field(j, "uid", topo_path);
        sw.pid = j.at("pID").get<std::string>();
        sw.station_uid = station_uid_from_object_uid(sw.uid);
        sw.type_id = j.value("typeID", "");
        sw.trunk = parse_switch_leg(j.at("trunk"), topo_path);
        sw.straight = parse_switch_leg(j.at("straight"), topo_path);
        sw.divergent = parse_switch_leg(j.at("divergent"), topo_path);
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
            sig.uid = require_uid_field(j, "uid", objects_path);
            sig.pid = j.at("pID").get<std::string>();
            sig.station_uid = station_uid_from_object_uid(sig.uid);
            sig.type_id = j.value("typeID", "");
            sig.type = parse_signal_type(j.at("type").get<std::string>());
            sig.governs_section_uid = require_uid_field(j, "governs_section", objects_path);
            sig.current_aspect = parse_initial_aspect(j.value("initial_aspect", "STOP"));
            state.insert_signal(sig);
        }

        for (const auto& j : objs.value("derailers", json::array()))
        {
            Derailer der;
            der.uid = require_uid_field(j, "uid", objects_path);
            der.pid = j.at("pID").get<std::string>();
            der.station_uid = station_uid_from_object_uid(der.uid);
            der.type_id = j.value("typeID", "");
            der.guards_section_uid = require_uid_field(j, "guards_section", objects_path);
            der.state = DerailerState::LOCKED;
            state.insert_derailer(der);
        }
    }

    return result;
}

std::vector<ScenarioMeta> load_world(EngineState& state,
                                     const std::vector<std::filesystem::path>& scenario_dirs)
{
    if (scenario_dirs.empty())
    {
        throw std::runtime_error("load_world: scenario_dirs must not be empty");
    }

    std::vector<ScenarioMeta> results;
    results.reserve(scenario_dirs.size());
    for (const auto& dir : scenario_dirs)
    {
        results.push_back(load_scenario(state, dir));
    }

    // The first scenario is primary: restore its session id, since every
    // load_scenario() call above overwrote it with its own station_sid.
    state.set_session_id(results.front().station_sid);

    return results;
}

}  // namespace engine::core
