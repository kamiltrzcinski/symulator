// tests/libscenario_validation/test_layer2.cpp
//
// Unit tests for Layer2Validator.
// Uses the real gdynia_orlowo scenario as the "happy path" reference.

#include "scenario_validation/layer2_validator.hpp"

#include <nlohmann/json.hpp>

#include <gtest/gtest.h>

#include <fstream>
#include <string>

using namespace scenario_validation;
using json = nlohmann::json;

// ── Helpers ───────────────────────────────────────────────────────────────────

static json load_json(const std::string& path)
{
    std::ifstream f{path};
    if (!f.is_open())
        throw std::runtime_error{"Cannot open: " + path};
    return json::parse(f);
}

static bool has_code(const ValidationResult& r, const std::string& code)
{
    for (const auto& i : r.issues)
        if (i.code == code)
            return true;
    return false;
}

// ── Happy path ────────────────────────────────────────────────────────────────

class Layer2GdyniaOrlowo : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* sd = std::getenv("SCENARIO_DIR");
        const std::string dir = sd ? sd : SCENARIO_DIR;
        topo_ = load_json(dir + "/topology.json");
        objs_ = load_json(dir + "/objects.json");
    }

    json topo_;
    json objs_;
    const std::string kSid = "GOr";
};

TEST_F(Layer2GdyniaOrlowo, ValidScenario_NoErrors)
{
    Layer2Validator v;
    const auto result = v.validate(kSid, topo_, objs_);
    EXPECT_TRUE(result.ok()) << "Expected 0 errors on valid gdynia_orlowo scenario";
    for (const auto& i : result.issues)
        if (i.severity == ValidationIssue::Severity::ERROR)
            ADD_FAILURE() << "[" << i.code << "] " << i.message;
}

// ── L2-001: sID mismatch ────────────────────────────────────────────────────

TEST(Layer2Validator, SidMismatch_ReportsL2001)
{
    json topo = {
        {"boundary_nodes", {{{"gID", "BND-A"}, {"pID", "p1"}, {"sID", "WRONG_SID"}}}},  // wrong sID
        {"track_sections", json::array()},
        {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer2Validator v;
    const auto result = v.validate("GOr", topo, objs);
    EXPECT_TRUE(has_code(result, "L2-001"));
}

// ── L2-002: OT references unknown ZWR ──────────────────────────────────────

TEST(Layer2Validator, OtReferencesUnknownZwr_ReportsL2002)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {{{"gID", "OT-T1"},
                    {"pID", "t1"},
                    {"sID", "S1"},
                    {"sideA", {{"neighborID", "BND-A"}}},
                    {"sideB", {{"neighborID", "ZWR-NONEXISTENT"}}}}}},  // no such ZWR
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer2Validator v;
    const auto result = v.validate("S1", topo, objs);
    EXPECT_TRUE(has_code(result, "L2-002"));
}

// ── L2-003: OT references unknown BND ──────────────────────────────────────

TEST(Layer2Validator, OtReferencesUnknownBnd_ReportsL2003)
{
    json topo = {{"boundary_nodes", json::array()},  // no BND-A defined
                 {"track_sections",
                  {{{"gID", "OT-T1"},
                    {"pID", "t1"},
                    {"sID", "S1"},
                    {"sideA", {{"neighborID", "BND-A"}}},  // not in boundary_nodes
                    {"sideB", {{"neighborID", "BND-B"}}}}}},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer2Validator v;
    const auto result = v.validate("S1", topo, objs);
    EXPECT_TRUE(has_code(result, "L2-003"));
}

// ── L2-004: ZWR leg references unknown OT ──────────────────────────────────

TEST(Layer2Validator, ZwrLegReferencesUnknownOt_ReportsL2004)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections", json::array()},  // no OTs defined
                 {"switches",
                  {{{"gID", "ZWR-A"},
                    {"pID", "p1"},
                    {"sID", "S1"},
                    {"trunk", {{"neighborID", "OT-GHOST"}, {"izID", "IZ-X"}}}}}}};
    json objs = {{"signals", json::array()}};

    Layer2Validator v;
    const auto result = v.validate("S1", topo, objs);
    EXPECT_TRUE(has_code(result, "L2-004"));
}

// ── L2-005: signal governs OT from different station ────────────────────────

TEST(Layer2Validator, SignalCrossStationRef_ReportsL2005)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {{{"gID", "OT-T1"},
                    {"pID", "t1"},
                    {"sID", "STATION_B"},  // different station
                    {"sideA", {{"neighborID", "BND-A"}}},
                    {"sideB", {{"neighborID", "BND-B"}}}}}},
                 {"switches", json::array()}};
    json objs = {{"signals",
                  {{{"gID", "SEM-1"},
                    {"pID", "p1"},
                    {"sID", "STATION_A"},
                    {"governs_track_section", "OT-T1"},
                    {"initial_aspect", "STOP"}}}}};

    Layer2Validator v;
    const auto result = v.validate("STATION_A", topo, objs);
    EXPECT_TRUE(has_code(result, "L2-005"));
}
