// engine/src/topology_loader.cpp

#include "engine/core/topology_loader.hpp"
#include "engine/core/track_model.hpp"
#include "engine/core/types.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <span>
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

BlockSectionState parse_block_state(const std::string& s)
{
    if (s == "CLOSED")
        return BlockSectionState::CLOSED;
    if (s == "OPEN")
        return BlockSectionState::OPEN;
    throw std::runtime_error("topology_loader: unknown initialState '" + s + "'");
}

BlockDirectionState parse_block_direction(const std::string& s)
{
    if (s == "NEUTRAL")
        return BlockDirectionState::NEUTRAL;
    if (s == "OUTBOUND")
        return BlockDirectionState::OUTBOUND;
    if (s == "INBOUND")
        return BlockDirectionState::INBOUND;
    throw std::runtime_error("topology_loader: unknown initialDirection '" + s + "'");
}

// ── Per-object loaders ────────────────────────────────────────────────────────
// One function per JSON object kind.  Adding a new kind = one loader function
// + one row in the registry tables below; load_scenario() never changes.

void load_boundary_node(EngineState& state, const json& j, const std::filesystem::path& path)
{
    BoundaryNode bn;
    bn.uid = require_uid_field(j, "uid", path);
    bn.pid = j.at("pID").get<std::string>();
    bn.station_uid = station_uid_from_object_uid(bn.uid);
    bn.description = j.value("description", "");
    state.insert_boundary_node(bn);
}

void load_track_section(EngineState& state, const json& j, const std::filesystem::path& path)
{
    TrackSection ts;
    ts.uid = require_uid_field(j, "uid", path);
    ts.pid = j.at("pID").get<std::string>();
    ts.station_uid = station_uid_from_object_uid(ts.uid);
    ts.side_a = parse_track_port(j.at("sideA"), path);
    ts.side_b = parse_track_port(j.at("sideB"), path);
    ts.length_m = j.value("lengthM", 0.0f);
    ts.electrified = j.value("electrified", false);
    ts.max_speed_kmh = j.value("maxSpeedKmh", 0);
    ts.occupancy = j.value("occupied", false) ? TrackOccupancy::OCCUPIED : TrackOccupancy::FREE;
    ts.station_section = j.value("station_section", true);
    state.insert_track_section(ts);
}

void load_switch(EngineState& state, const json& j, const std::filesystem::path& path)
{
    Switch sw;
    sw.uid = require_uid_field(j, "uid", path);
    sw.pid = j.at("pID").get<std::string>();
    sw.station_uid = station_uid_from_object_uid(sw.uid);
    sw.type_id = j.value("typeID", "");
    sw.trunk = parse_switch_leg(j.at("trunk"), path);
    sw.straight = parse_switch_leg(j.at("straight"), path);
    sw.divergent = parse_switch_leg(j.at("divergent"), path);
    sw.length_m = j.value("lengthM", 0.0f);
    sw.max_speed_straight_kmh = j.value("maxSpeedStraightKmh", 0);
    sw.max_speed_divergent_kmh = j.value("maxSpeedDivergentKmh", 0);
    state.insert_switch(sw);
}

void load_block_section(EngineState& state, const json& j, const std::filesystem::path& path)
{
    BlockSection bs;
    bs.uid = require_uid_field(j, "uid", path);
    bs.pid = j.at("pID").get<std::string>();
    bs.type_id = j.value("type_id", "SHL-12");
    bs.station_uid = station_uid_from_object_uid(bs.uid);
    bs.neighbor_station_uid = require_uid_field(j, "neighborStationUID", path);
    bs.line_number = j.value("lineNumber", 0);
    bs.departure_signal_uid = require_uid_field(j, "departureSignalUID", path);
    bs.entry_signal_uid = require_uid_field(j, "entrySignalUID", path);
    // szlakSectionUIDs may reference sections of a neighbouring scenario that
    // is not loaded — no cross-validation here (Model A graceful fallback).
    for (const auto& s : j.value("szlakSectionUIDs", json::array()))
        bs.szlak_section_uids.push_back(UID{s.get<std::uint64_t>()});
    bs.state = parse_block_state(j.value("initialState", "CLOSED"));
    bs.direction = parse_block_direction(j.value("initialDirection", "NEUTRAL"));
    state.insert_block_section(bs);
}

void load_signal(EngineState& state, const json& j, const std::filesystem::path& path)
{
    Signal sig;
    sig.uid = require_uid_field(j, "uid", path);
    sig.pid = j.at("pID").get<std::string>();
    sig.station_uid = station_uid_from_object_uid(sig.uid);
    sig.type_id = j.value("typeID", "");
    sig.type = parse_signal_type(j.at("type").get<std::string>());
    sig.governs_section_uid = require_uid_field(j, "governs_section", path);
    sig.current_aspect = parse_initial_aspect(j.value("initial_aspect", "STOP"));
    state.insert_signal(sig);
}

void load_derailer(EngineState& state, const json& j, const std::filesystem::path& path)
{
    Derailer der;
    der.uid = require_uid_field(j, "uid", path);
    der.pid = j.at("pID").get<std::string>();
    der.station_uid = station_uid_from_object_uid(der.uid);
    der.type_id = j.value("typeID", "");
    der.guards_section_uid = require_uid_field(j, "guards_section", path);
    der.state = DerailerState::LOCKED;
    state.insert_derailer(der);
}

// ── Object-kind registries ────────────────────────────────────────────────────

using ObjectLoader = void (*)(EngineState&, const json&, const std::filesystem::path&);

struct ObjectKind
{
    const char* json_key;
    ObjectLoader load;
};

constexpr ObjectKind kTopologyKinds[] = {
    {"boundary_nodes", load_boundary_node},
    {"track_sections", load_track_section},
    {"switches", load_switch},
    {"block_sections", load_block_section},
};

constexpr ObjectKind kObjectsKinds[] = {
    {"signals", load_signal},
    {"derailers", load_derailer},
};

void load_object_arrays(EngineState& state, const json& doc, const std::filesystem::path& path,
                        std::span<const ObjectKind> kinds)
{
    for (const auto& kind : kinds)
        for (const auto& j : doc.value(kind.json_key, json::array()))
            kind.load(state, j, path);
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
    const auto topo_path = dir / "topology.json";
    load_object_arrays(state, read_json(topo_path), topo_path, kTopologyKinds);

    // ── objects.json (optional) ──────────────────────────────────────────────
    const auto objects_path = dir / "objects.json";
    if (std::filesystem::exists(objects_path))
        load_object_arrays(state, read_json(objects_path), objects_path, kObjectsKinds);

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
