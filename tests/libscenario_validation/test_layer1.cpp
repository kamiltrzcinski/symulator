// tests/libscenario_validation/test_layer1.cpp
//
// Unit tests for Layer1Validator.
// Uses the real gdynia_orlowo scenario as the "happy path" reference.
// Broken JSON fixtures test individual error codes.

#include "scenario_validation/layer1_validator.hpp"

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

// Has at least one issue with the given code.
static bool has_code(const ValidationResult& r, const std::string& code)
{
    for (const auto& i : r.issues)
        if (i.code == code)
            return true;
    return false;
}

// ── Happy path ────────────────────────────────────────────────────────────────

class Layer1GdyniaOrlowo : public ::testing::Test
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
};

TEST_F(Layer1GdyniaOrlowo, ValidScenario_NoErrors)
{
    Layer1Validator v;
    const auto result = v.validate(topo_, objs_);
    EXPECT_TRUE(result.ok()) << "Expected 0 errors on valid gdynia_orlowo scenario";
    for (const auto& i : result.issues)
        if (i.severity == ValidationIssue::Severity::ERROR)
            ADD_FAILURE() << "[" << i.code << "] " << i.message;
}

TEST_F(Layer1GdyniaOrlowo, ValidScenario_NoDuplicateGIDs)
{
    Layer1Validator v;
    const auto result = v.validate(topo_, objs_);
    EXPECT_FALSE(has_code(result, "L1-002"));
}

// ── L1-001: missing required field ───────────────────────────────────────────

TEST(Layer1Validator, MissingGID_ReportsL1001)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {
                      {{"pID", "abc"},
                       {"sID", "XXX"},  // gID absent
                       {"sideA", {{"neighborID", "BND-A"}}},
                       {"sideB", {{"neighborID", "BND-B"}}}},
                  }},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-001"));
}

TEST(Layer1Validator, MissingSID_ReportsL1001)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {
                      {{"gID", "OT-X"},
                       {"pID", "abc"},  // sID absent
                       {"sideA", {{"neighborID", "BND-A"}}},
                       {"sideB", {{"neighborID", "BND-B"}}}},
                  }},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-001"));
}

// ── L1-002: duplicate GID ──────────────────────────────────────────────────

TEST(Layer1Validator, DuplicateGID_ReportsL1002)
{
    json topo = {{"boundary_nodes",
                  {{{"gID", "BND-A"}, {"pID", "p1"}, {"sID", "S1"}},
                   {{"gID", "BND-A"}, {"pID", "p2"}, {"sID", "S1"}}}},  // duplicate!
                 {"track_sections", json::array()},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-002"));
}

// ── L1-003: unknown izID ───────────────────────────────────────────────────

TEST(Layer1Validator, UnknownIzID_ReportsL1003)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {{{"gID", "OT-X"},
                    {"pID", "px"},
                    {"sID", "S1"},
                    {"sideA", {{"neighborID", "BND-NONE"}}},
                    {"sideB", {{"neighborID", "ZWR-Y"}, {"izID", "IZ-NONEXISTENT"}}}}}},
                 // No ZWR in switches that defines IZ-NONEXISTENT.
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-003"));
}

// ── L1-004: signal in OT.signals[] not in objects.json ────────────────────

TEST(Layer1Validator, OtReferencesUnknownSignal_ReportsL1004)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {{{"gID", "OT-T1"},
                    {"pID", "t1"},
                    {"sID", "S1"},
                    {"sideA", {{"neighborID", "BND-A"}, {"signals", {"SEM-GHOST"}}}},
                    {"sideB", {{"neighborID", "BND-B"}}}}}},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};  // no signals defined

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-004"));
}

// ── L1-005: signal governs unknown OT ────────────────────────────────────

TEST(Layer1Validator, SignalGovernsMissingOT_ReportsL1005)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections", json::array()},
                 {"switches", json::array()}};
    json objs = {{"signals",
                  {{{"gID", "SEM-1"},
                    {"pID", "p1"},
                    {"sID", "S1"},
                    {"governs_track_section", "OT-GHOST"},
                    {"initial_aspect", "STOP"}}}}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-005"));
}

// ── L1-006: invalid initial_aspect ──────────────────────────────────────

TEST(Layer1Validator, InvalidAspect_ReportsL1006)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {{{"gID", "OT-T1"},
                    {"pID", "t1"},
                    {"sID", "S1"},
                    {"sideA", {{"neighborID", "BND-A"}}},
                    {"sideB", {{"neighborID", "BND-B"}}}}}},
                 {"switches", json::array()}};
    json objs = {{"signals",
                  {{
                      {"gID", "SEM-1"},
                      {"pID", "p1"},
                      {"sID", "S1"},
                      {"governs_track_section", "OT-T1"},
                      {"initial_aspect", "SUPER_GREEN"}  // invalid!
                  }}}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-006"));
}
