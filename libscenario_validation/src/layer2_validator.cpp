// libscenario_validation/src/layer2_validator.cpp

#include "scenario_validation/layer2_validator.hpp"

#include <unordered_set>

namespace scenario_validation
{

namespace
{

using Severity = ValidationIssue::Severity;

std::unordered_set<std::string> collect_gids(const nlohmann::json& arr)
{
    std::unordered_set<std::string> out;
    for (const auto& obj : arr)
        if (obj.contains("gID"))
            out.insert(obj["gID"].get<std::string>());
    return out;
}

// Check sID of every object in the array against the expected station_sid.
void check_sids(const nlohmann::json& arr, const std::string& expected_sid,
                const std::string& array_name, ValidationResult& result)
{
    for (const auto& obj : arr)
    {
        if (!obj.contains("sID"))
            continue;
        const auto sid = obj["sID"].get<std::string>();
        if (sid != expected_sid)
        {
            const std::string gid =
                obj.contains("gID") ? obj["gID"].get<std::string>() : "(unknown)";
            result.issues.push_back({Severity::ERROR, "L2-001",
                                     array_name + " " + gid + ": sID '" + sid +
                                         "' does not match station_sid '" + expected_sid + "'",
                                     gid});
        }
    }
}

}  // anonymous namespace

// ── Layer2Validator ───────────────────────────────────────────────────────────

ValidationResult Layer2Validator::validate(const std::string& station_sid,
                                           const nlohmann::json& topology,
                                           const nlohmann::json& objects) const
{
    ValidationResult result;

    const auto ot_gids = collect_gids(topology.value("track_sections", nlohmann::json::array()));
    const auto bnd_gids = collect_gids(topology.value("boundary_nodes", nlohmann::json::array()));
    const auto zwr_gids = collect_gids(topology.value("switches", nlohmann::json::array()));

    // 1. sID consistency across all arrays.
    check_sids(topology.value("boundary_nodes", nlohmann::json::array()), station_sid,
               "boundary_node", result);
    check_sids(topology.value("track_sections", nlohmann::json::array()), station_sid,
               "track_section", result);
    check_sids(topology.value("switches", nlohmann::json::array()), station_sid, "switch", result);
    check_sids(objects.value("signals", nlohmann::json::array()), station_sid, "signal", result);

    // 2. OT neighborID references.
    for (const auto& ot : topology.value("track_sections", nlohmann::json::array()))
    {
        const std::string gid = ot.contains("gID") ? ot["gID"].get<std::string>() : "(unknown)";

        for (const auto* side : {"sideA", "sideB"})
        {
            if (!ot.contains(side))
                continue;
            const auto& s = ot[side];
            if (!s.contains("neighborID"))
                continue;

            const auto neighbor = s["neighborID"].get<std::string>();
            if (neighbor.rfind("ZWR-", 0) == 0)
            {
                if (zwr_gids.find(neighbor) == zwr_gids.end())
                {
                    result.issues.push_back({Severity::ERROR, "L2-002",
                                             "track_section " + gid + " " + side + ".neighborID '" +
                                                 neighbor + "' references unknown switch",
                                             gid});
                }
            }
            else if (neighbor.rfind("BND-", 0) == 0)
            {
                if (bnd_gids.find(neighbor) == bnd_gids.end())
                {
                    result.issues.push_back({Severity::ERROR, "L2-003",
                                             "track_section " + gid + " " + side + ".neighborID '" +
                                                 neighbor + "' references unknown boundary_node",
                                             gid});
                }
            }
        }
    }

    // 3. ZWR leg neighborID references must be existing OTs.
    for (const auto& sw : topology.value("switches", nlohmann::json::array()))
    {
        const std::string gid = sw.contains("gID") ? sw["gID"].get<std::string>() : "(unknown)";

        for (const auto* leg : {"trunk", "straight", "divergent"})
        {
            if (!sw.contains(leg))
                continue;
            const auto& l = sw[leg];
            if (!l.contains("neighborID"))
                continue;

            const auto neighbor = l["neighborID"].get<std::string>();
            if (!neighbor.empty() && ot_gids.find(neighbor) == ot_gids.end())
            {
                result.issues.push_back({Severity::ERROR, "L2-004",
                                         "switch " + gid + " " + leg + ".neighborID '" + neighbor +
                                             "' references unknown track_section",
                                         gid});
            }
        }
    }

    // 4. Signal governs_track_section — must be in same station (sID check already covers OT side).
    //    Here we check the OT actually exists (L1-005 covers the existence itself; L2-005 is
    //    for cross-station reference).
    for (const auto& sig : objects.value("signals", nlohmann::json::array()))
    {
        if (!sig.contains("governs_track_section") || !sig.contains("sID"))
            continue;

        const std::string gid = sig.contains("gID") ? sig["gID"].get<std::string>() : "(unknown)";
        const auto ot_ref = sig["governs_track_section"].get<std::string>();
        const auto sig_sid = sig["sID"].get<std::string>();

        // Find the referenced OT's sID.
        for (const auto& ot : topology.value("track_sections", nlohmann::json::array()))
        {
            if (!ot.contains("gID") || ot["gID"].get<std::string>() != ot_ref)
                continue;
            if (ot.contains("sID") && ot["sID"].get<std::string>() != sig_sid)
            {
                result.issues.push_back(
                    {Severity::ERROR, "L2-005",
                     "signal " + gid + " governs OT '" + ot_ref + "' from different station ('" +
                         ot["sID"].get<std::string>() + "' vs '" + sig_sid + "')",
                     gid});
            }
            break;
        }
    }

    return result;
}

}  // namespace scenario_validation
