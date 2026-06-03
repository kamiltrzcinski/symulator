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
using json = nlohmann::json;

constexpr uint64_t MAX_SAFE_JSON_INT = (1ULL << 53) - 1ULL;

// ── Helpers ───────────────────────────────────────────────────────────────────

// Check that "uid" is a positive integer <= 2^53-1 and "pID" is a non-empty string.
void check_required_fields(const json& obj, const std::string& context, ValidationResult& result)
{
    if (!obj.contains("uid") || !obj["uid"].is_number_unsigned() ||
        obj["uid"].get<std::uint64_t>() == 0 || obj["uid"].get<std::uint64_t>() > MAX_SAFE_JSON_INT)
    {
        result.issues.push_back(
            {Severity::ERROR, "L1-001",
             context + ": required field 'uid' is missing, zero, or exceeds 2^53-1", std::nullopt});
    }

    if (!obj.contains("pID") || !obj["pID"].is_string() || obj["pID"].get<std::string>().empty())
    {
        result.issues.push_back({Severity::ERROR, "L1-001",
                                 context + ": required field 'pID' is missing or empty",
                                 std::nullopt});
    }
}

void check_duplicate_uids(const std::vector<std::uint64_t>& uids, ValidationResult& result)
{
    std::unordered_set<std::uint64_t> seen;
    for (const auto uid : uids)
    {
        if (uid == 0)
            continue;
        if (!seen.insert(uid).second)
        {
            result.issues.push_back(
                {Severity::ERROR, "L1-002", "Duplicate UID: " + std::to_string(uid), std::nullopt});
        }
    }
}

// Collect all IZ UIDs defined by all switch entries (izUID fields in legs).
std::unordered_set<std::uint64_t> collect_switch_iz_uids(const json& topology)
{
    std::unordered_set<std::uint64_t> iz_uids;
    for (const auto& sw : topology.value("switches", json::array()))
    {
        for (const auto* leg : {"trunk", "straight", "divergent"})
        {
            if (sw.contains(leg) && sw[leg].contains("izUID"))
            {
                const auto iz = sw[leg]["izUID"].get<std::uint64_t>();
                if (iz != 0)
                    iz_uids.insert(iz);
            }
        }
    }
    return iz_uids;
}

// Collect track section UIDs.
std::unordered_set<std::uint64_t> collect_section_uids(const json& topology)
{
    std::unordered_set<std::uint64_t> uids;
    for (const auto& ot : topology.value("track_sections", json::array()))
    {
        if (ot.contains("uid") && ot["uid"].is_number_unsigned())
            uids.insert(ot["uid"].get<std::uint64_t>());
    }
    return uids;
}

// Collect signal UIDs from objects.json.
std::unordered_set<std::uint64_t> collect_signal_uids(const json& objects)
{
    std::unordered_set<std::uint64_t> uids;
    for (const auto& sig : objects.value("signals", json::array()))
    {
        if (sig.contains("uid") && sig["uid"].is_number_unsigned())
            uids.insert(sig["uid"].get<std::uint64_t>());
    }
    return uids;
}

static const std::unordered_set<std::string> kValidAspects{
    "STOP",     "S1_STOP", "CAUTION",    "PROCEED",       "CLEAR",    "CAUTION_THEN_STOP",
    "SHUNTING", "REPEAT",  "S2_PROCEED", "S3_PROCEED_40", "MS1_STOP", "MS2_SHUNTING_ALLOWED"};

}  // anonymous namespace

// ── Layer1Validator ───────────────────────────────────────────────────────────

ValidationResult Layer1Validator::validate(const nlohmann::json& topology,
                                           const nlohmann::json& objects) const
{
    ValidationResult result;

    std::vector<std::uint64_t> all_uids;

    // 1. Boundary nodes — required fields + UID collection.
    for (const auto& node : topology.value("boundary_nodes", json::array()))
    {
        check_required_fields(node, "boundary_node", result);
        if (node.contains("uid") && node["uid"].is_number_unsigned())
            all_uids.push_back(node["uid"].get<std::uint64_t>());
    }

    // 2. Track sections — required fields + izUID references + signal UID refs.
    const auto switch_iz_uids = collect_switch_iz_uids(topology);
    const auto sig_uids = collect_signal_uids(objects);

    for (const auto& ot : topology.value("track_sections", json::array()))
    {
        check_required_fields(ot, "track_section", result);
        const uint64_t uid = (ot.contains("uid") && ot["uid"].is_number_unsigned())
                                 ? ot["uid"].get<std::uint64_t>()
                                 : 0;
        all_uids.push_back(uid);

        for (const auto* side : {"sideA", "sideB"})
        {
            if (!ot.contains(side))
                continue;
            const auto& s = ot[side];

            // Check izUID references on track port (switch-side counter).
            if (s.contains("izUID") && s["izUID"].is_number_unsigned())
            {
                const auto iz = s["izUID"].get<std::uint64_t>();
                if (iz != 0 && switch_iz_uids.find(iz) == switch_iz_uids.end())
                {
                    result.issues.push_back({Severity::ERROR, "L1-003",
                                             "TrackSection uid=" + std::to_string(uid) + " " +
                                                 side + ".izUID=" + std::to_string(iz) +
                                                 " does not match any switch leg Iz counter",
                                             std::nullopt});
                }
            }

            // Check signalUIDs references.
            if (s.contains("signalUIDs"))
            {
                for (const auto& sig_uid_json : s["signalUIDs"])
                {
                    if (!sig_uid_json.is_number_unsigned())
                        continue;
                    const auto sig_uid = sig_uid_json.get<std::uint64_t>();
                    if (sig_uid != 0 && sig_uids.find(sig_uid) == sig_uids.end())
                    {
                        result.issues.push_back(
                            {Severity::ERROR, "L1-004",
                             "TrackSection uid=" + std::to_string(uid) + " " + side +
                                 ".signalUIDs[] references unknown signal uid=" +
                                 std::to_string(sig_uid),
                             std::nullopt});
                    }
                }
            }
        }
    }

    // 3. Switches — required fields + UID collection.
    for (const auto& sw : topology.value("switches", json::array()))
    {
        check_required_fields(sw, "switch", result);
        if (sw.contains("uid") && sw["uid"].is_number_unsigned())
            all_uids.push_back(sw["uid"].get<std::uint64_t>());
    }

    // 4. Signals in objects.json — required fields + governs_section + initial_aspect.
    const auto section_uids = collect_section_uids(topology);

    for (const auto& sig : objects.value("signals", json::array()))
    {
        check_required_fields(sig, "signal", result);
        const uint64_t uid = (sig.contains("uid") && sig["uid"].is_number_unsigned())
                                 ? sig["uid"].get<std::uint64_t>()
                                 : 0;
        all_uids.push_back(uid);

        if (sig.contains("governs_section"))
        {
            const auto ot_uid = sig["governs_section"].get<std::uint64_t>();
            if (ot_uid != 0 && section_uids.find(ot_uid) == section_uids.end())
            {
                result.issues.push_back({Severity::ERROR, "L1-005",
                                         "Signal uid=" + std::to_string(uid) +
                                             " governs_section=" + std::to_string(ot_uid) +
                                             " does not reference a known track_section",
                                         std::nullopt});
            }
        }

        if (sig.contains("initial_aspect"))
        {
            const auto aspect = sig["initial_aspect"].get<std::string>();
            if (kValidAspects.find(aspect) == kValidAspects.end())
            {
                result.issues.push_back({Severity::ERROR, "L1-006",
                                         "Signal uid=" + std::to_string(uid) +
                                             " has invalid initial_aspect '" + aspect + "'",
                                         std::nullopt});
            }
        }
    }

    // 5. Duplicate UID check.
    check_duplicate_uids(all_uids, result);

    return result;
}

}  // namespace scenario_validation
