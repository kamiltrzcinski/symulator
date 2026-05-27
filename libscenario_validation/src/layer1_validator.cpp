// libscenario_validation/src/layer1_validator.cpp

#include "scenario_validation/layer1_validator.hpp"

#include <set>
#include <string>
#include <unordered_set>

namespace scenario_validation
{

namespace
{

using Severity = ValidationIssue::Severity;

// ── Helpers ───────────────────────────────────────────────────────────────────

void check_required_fields(const nlohmann::json& obj, const std::string& context,
                           ValidationResult& result)
{
    for (const auto& field : {"gID", "pID", "sID"})
    {
        if (!obj.contains(field) || !obj[field].is_string() ||
            obj[field].get<std::string>().empty())
        {
            result.issues.push_back(
                {Severity::ERROR, "L1-001",
                 context + ": required field '" + field + "' is missing or empty", std::nullopt});
        }
    }
}

void check_duplicate_gids(const std::vector<std::string>& gids, ValidationResult& result)
{
    std::unordered_set<std::string> seen;
    for (const auto& gid : gids)
    {
        if (!seen.insert(gid).second)
        {
            result.issues.push_back({Severity::ERROR, "L1-002", "Duplicate GID: " + gid, gid});
        }
    }
}

// Collect all Iz IDs defined by all ZWR entries in the topology.
std::unordered_set<std::string> collect_zwr_iz_ids(const nlohmann::json& topology)
{
    std::unordered_set<std::string> iz_ids;
    const auto& switches = topology.value("switches", nlohmann::json::array());
    for (const auto& sw : switches)
    {
        for (const auto& leg : {"trunk", "straight", "divergent"})
        {
            if (sw.contains(leg) && sw[leg].contains("izID"))
            {
                const auto iz = sw[leg]["izID"].get<std::string>();
                if (!iz.empty())
                    iz_ids.insert(iz);
            }
        }
    }
    return iz_ids;
}

// Collect all OT GIDs from topology.
std::unordered_set<std::string> collect_ot_gids(const nlohmann::json& topology)
{
    std::unordered_set<std::string> gids;
    for (const auto& ot : topology.value("track_sections", nlohmann::json::array()))
    {
        if (ot.contains("gID"))
            gids.insert(ot["gID"].get<std::string>());
    }
    return gids;
}

// Collect all signal GIDs from objects.json.
std::unordered_set<std::string> collect_signal_gids(const nlohmann::json& objects)
{
    std::unordered_set<std::string> gids;
    for (const auto& sig : objects.value("signals", nlohmann::json::array()))
    {
        if (sig.contains("gID"))
            gids.insert(sig["gID"].get<std::string>());
    }
    return gids;
}

static const std::unordered_set<std::string> kValidAspects{
    "STOP", "CAUTION", "PROCEED", "CLEAR", "CAUTION_THEN_STOP", "SHUNTING", "REPEAT"};

}  // anonymous namespace

// ── Layer1Validator ───────────────────────────────────────────────────────────

ValidationResult Layer1Validator::validate(const nlohmann::json& topology,
                                           const nlohmann::json& objects) const
{
    ValidationResult result;

    // Collect all GIDs for duplicate detection.
    std::vector<std::string> all_gids;

    // 1. Boundary nodes — required fields + GID collection.
    for (const auto& node : topology.value("boundary_nodes", nlohmann::json::array()))
    {
        check_required_fields(node, "boundary_node", result);
        if (node.contains("gID"))
            all_gids.push_back(node["gID"].get<std::string>());
    }

    // 2. Track sections (OTs) — required fields + izID references + signal refs.
    const auto zwr_iz_ids = collect_zwr_iz_ids(topology);
    const auto signal_gids = collect_signal_gids(objects);

    for (const auto& ot : topology.value("track_sections", nlohmann::json::array()))
    {
        check_required_fields(ot, "track_section", result);
        const std::string gid = ot.contains("gID") ? ot["gID"].get<std::string>() : "(unknown)";
        all_gids.push_back(gid);

        // Check izID references on sideA and sideB.
        for (const auto* side : {"sideA", "sideB"})
        {
            if (!ot.contains(side))
                continue;
            const auto& s = ot[side];

            if (s.contains("izID"))
            {
                const auto iz_id = s["izID"].get<std::string>();
                if (!iz_id.empty() && zwr_iz_ids.find(iz_id) == zwr_iz_ids.end())
                {
                    result.issues.push_back({Severity::ERROR, "L1-003",
                                             "OT " + gid + " " + side + ".izID '" + iz_id +
                                                 "' does not match any ZWR Iz zone",
                                             gid});
                }
            }

            // Check signal GID references.
            if (s.contains("signals"))
            {
                for (const auto& sig_gid_json : s["signals"])
                {
                    const auto sig_gid = sig_gid_json.get<std::string>();
                    if (!sig_gid.empty() && signal_gids.find(sig_gid) == signal_gids.end())
                    {
                        result.issues.push_back({Severity::ERROR, "L1-004",
                                                 "OT " + gid + " " + side +
                                                     ".signals[] references unknown signal '" +
                                                     sig_gid + "'",
                                                 gid});
                    }
                }
            }
        }
    }

    // 3. Switches (ZWRs) — required fields + GID collection.
    for (const auto& sw : topology.value("switches", nlohmann::json::array()))
    {
        check_required_fields(sw, "switch", result);
        if (sw.contains("gID"))
            all_gids.push_back(sw["gID"].get<std::string>());
    }

    // 4. Signals in objects.json — required fields + governs_track_section + initial_aspect.
    const auto ot_gids = collect_ot_gids(topology);

    for (const auto& sig : objects.value("signals", nlohmann::json::array()))
    {
        check_required_fields(sig, "signal", result);
        const std::string gid = sig.contains("gID") ? sig["gID"].get<std::string>() : "(unknown)";
        all_gids.push_back(gid);

        // governs_track_section must reference a known OT.
        if (sig.contains("governs_track_section"))
        {
            const auto ot_ref = sig["governs_track_section"].get<std::string>();
            if (!ot_ref.empty() && ot_gids.find(ot_ref) == ot_gids.end())
            {
                result.issues.push_back({Severity::ERROR, "L1-005",
                                         "Signal " + gid + " governs_track_section '" + ot_ref +
                                             "' does not reference a known track_section",
                                         gid});
            }
        }

        // initial_aspect must be a known value.
        if (sig.contains("initial_aspect"))
        {
            const auto aspect = sig["initial_aspect"].get<std::string>();
            if (kValidAspects.find(aspect) == kValidAspects.end())
            {
                result.issues.push_back(
                    {Severity::ERROR, "L1-006",
                     "Signal " + gid + " has invalid initial_aspect '" + aspect + "'", gid});
            }
        }
    }

    // 5. Duplicate GID check across all collected GIDs.
    check_duplicate_gids(all_gids, result);

    return result;
}

}  // namespace scenario_validation
