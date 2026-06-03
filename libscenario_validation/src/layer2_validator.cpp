// libscenario_validation/src/layer2_validator.cpp

#include "scenario_validation/layer2_validator.hpp"

#include <unordered_set>

namespace scenario_validation
{

namespace
{

using Severity = ValidationIssue::Severity;
using json = nlohmann::json;

// Extract SCOPE field (bits 31-16) from a UID value.
static uint16_t uid_scope(uint64_t uid)
{
    return static_cast<uint16_t>((uid >> 16) & 0xFFFF);
}

std::unordered_set<uint64_t> collect_uids(const json& arr)
{
    std::unordered_set<uint64_t> out;
    for (const auto& obj : arr)
        if (obj.contains("uid") && obj["uid"].is_number_unsigned())
            out.insert(obj["uid"].get<uint64_t>());
    return out;
}

}  // anonymous namespace

// ── Layer2Validator ───────────────────────────────────────────────────────────

ValidationResult Layer2Validator::validate(int station_instance, const json& topology,
                                           const json& objects) const
{
    ValidationResult result;

    const auto ot_uids = collect_uids(topology.value("track_sections", json::array()));
    const auto bnd_uids = collect_uids(topology.value("boundary_nodes", json::array()));
    const auto zwr_uids = collect_uids(topology.value("switches", json::array()));

    // Union of all topology UIDs (used to check neighborUID for boundary-crossing refs).
    std::unordered_set<uint64_t> all_topo_uids;
    all_topo_uids.insert(ot_uids.begin(), ot_uids.end());
    all_topo_uids.insert(bnd_uids.begin(), bnd_uids.end());
    all_topo_uids.insert(zwr_uids.begin(), zwr_uids.end());

    // 1. SCOPE consistency: all topology UIDs must have the same station SCOPE.
    auto check_scope = [&](const json& arr, const std::string& kind)
    {
        for (const auto& obj : arr)
        {
            if (!obj.contains("uid") || !obj["uid"].is_number_unsigned())
                continue;
            const auto uid = obj["uid"].get<uint64_t>();
            const int scope = static_cast<int>(uid_scope(uid));
            if (scope != station_instance)
            {
                result.issues.push_back(
                    {Severity::ERROR, "L2-001",
                     kind + " uid=" + std::to_string(uid) + " has SCOPE=" + std::to_string(scope) +
                         " but expected station_instance=" + std::to_string(station_instance),
                     std::nullopt});
            }
        }
    };

    check_scope(topology.value("boundary_nodes", json::array()), "boundary_node");
    check_scope(topology.value("track_sections", json::array()), "track_section");
    check_scope(topology.value("switches", json::array()), "switch");
    check_scope(objects.value("signals", json::array()), "signal");

    // 2. Track section neighborUID references.
    for (const auto& ot : topology.value("track_sections", json::array()))
    {
        const uint64_t uid = ot.contains("uid") ? ot["uid"].get<uint64_t>() : 0;

        for (const auto* side : {"sideA", "sideB"})
        {
            if (!ot.contains(side))
                continue;
            const auto& s = ot[side];
            if (!s.contains("neighborUID") || !s["neighborUID"].is_number_unsigned())
                continue;

            const auto neighbor = s["neighborUID"].get<uint64_t>();
            if (neighbor == 0)
                continue;

            // Neighbor should be a switch (KIND=0x14) or boundary_node (KIND=0x18)
            const uint8_t kind = static_cast<uint8_t>((neighbor >> 32) & 0xFF);
            if (kind == 0x14)  // SWITCH
            {
                if (zwr_uids.find(neighbor) == zwr_uids.end())
                    result.issues.push_back({Severity::ERROR, "L2-002",
                                             "track_section uid=" + std::to_string(uid) + " " +
                                                 side + ".neighborUID=" + std::to_string(neighbor) +
                                                 " references unknown switch",
                                             std::nullopt});
            }
            else if (kind == 0x18)  // BOUNDARY_NODE
            {
                if (bnd_uids.find(neighbor) == bnd_uids.end())
                    result.issues.push_back({Severity::ERROR, "L2-003",
                                             "track_section uid=" + std::to_string(uid) + " " +
                                                 side + ".neighborUID=" + std::to_string(neighbor) +
                                                 " references unknown boundary_node",
                                             std::nullopt});
            }
        }
    }

    // 3. Switch leg neighborUID must be existing track sections or boundary nodes.
    for (const auto& sw : topology.value("switches", json::array()))
    {
        const uint64_t uid = sw.contains("uid") ? sw["uid"].get<uint64_t>() : 0;

        for (const auto* leg : {"trunk", "straight", "divergent"})
        {
            if (!sw.contains(leg))
                continue;
            const auto& l = sw[leg];
            if (!l.contains("neighborUID") || !l["neighborUID"].is_number_unsigned())
                continue;

            const auto neighbor = l["neighborUID"].get<uint64_t>();
            if (neighbor == 0)
                continue;

            if (ot_uids.find(neighbor) == ot_uids.end() &&
                bnd_uids.find(neighbor) == bnd_uids.end())
            {
                result.issues.push_back({Severity::ERROR, "L2-004",
                                         "switch uid=" + std::to_string(uid) + " " + leg +
                                             ".neighborUID=" + std::to_string(neighbor) +
                                             " references unknown track_section or boundary_node",
                                         std::nullopt});
            }
        }
    }

    // 4. Signal governs_section must be in track_sections.
    for (const auto& sig : objects.value("signals", json::array()))
    {
        const uint64_t uid = sig.contains("uid") ? sig["uid"].get<uint64_t>() : 0;
        if (!sig.contains("governs_section") || !sig["governs_section"].is_number_unsigned())
            continue;

        const auto ot_uid = sig["governs_section"].get<uint64_t>();
        if (ot_uid != 0 && ot_uids.find(ot_uid) == ot_uids.end())
        {
            result.issues.push_back({Severity::ERROR, "L2-005",
                                     "signal uid=" + std::to_string(uid) +
                                         " governs_section=" + std::to_string(ot_uid) +
                                         " references unknown track_section",
                                     std::nullopt});
        }
    }

    return result;
}

}  // namespace scenario_validation
