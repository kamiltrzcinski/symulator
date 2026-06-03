// tests/libscenario_validation/test_layer2.cpp
//
// Unit tests for Layer2Validator.
// Uses the real gdynia_orlowo scenario as the "happy path" reference.

#include "scenario_validation/layer2_validator.hpp"

#include <nlohmann/json.hpp>

#include <gtest/gtest.h>

#include <cstdint>
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

// Construct INFRA UID: domain=0x02, given kind, scope=station, instance=n
static constexpr uint64_t infra_uid(uint8_t kind, uint16_t station, uint16_t n)
{
    return (uint64_t{0x02} << 40) | (uint64_t{kind} << 32) | (uint64_t{station} << 16) |
           uint64_t{n};
}

static constexpr uint8_t BOUNDARY_NODE = 0x18;
static constexpr uint8_t TRACK_SECTION = 0x13;
static constexpr uint8_t SWITCH_KIND = 0x14;
static constexpr uint8_t SIGNAL_KIND = 0x15;

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
    static constexpr int kGOrInstance = 1;  // GOr = station instance 1
};

TEST_F(Layer2GdyniaOrlowo, ValidScenario_NoErrors)
{
    Layer2Validator v;
    const auto result = v.validate(kGOrInstance, topo_, objs_);
    EXPECT_TRUE(result.ok()) << "Expected 0 errors on valid gdynia_orlowo scenario";
    for (const auto& i : result.issues)
        if (i.severity == ValidationIssue::Severity::ERROR)
            ADD_FAILURE() << "[" << i.code << "] " << i.message;
}

// ── L2-001: SCOPE mismatch ────────────────────────────────────────────────────

TEST(Layer2Validator, ScopeMismatch_ReportsL2001)
{
    // station_instance=1 but UID has SCOPE=2 (wrong station)
    const uint64_t wrongUid = infra_uid(BOUNDARY_NODE, 2, 1);
    json topo = {{"boundary_nodes", {{{"uid", wrongUid}, {"pID", "p1"}}}},
                 {"track_sections", json::array()},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer2Validator v;
    const auto result = v.validate(1, topo, objs);
    EXPECT_TRUE(has_code(result, "L2-001"));
}

// ── L2-002: track section references unknown switch ───────────────────────────

TEST(Layer2Validator, OtReferencesUnknownSwitch_ReportsL2002)
{
    const uint64_t kSec = infra_uid(TRACK_SECTION, 1, 1);
    const uint64_t kBnd = infra_uid(BOUNDARY_NODE, 1, 1);
    const uint64_t kGhostSw = infra_uid(SWITCH_KIND, 1, 99);  // not in switches

    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {{{"uid", kSec},
                    {"pID", "t1"},
                    {"sideA", {{"neighborUID", kBnd}}},
                    {"sideB", {{"neighborUID", kGhostSw}}}}}},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer2Validator v;
    const auto result = v.validate(1, topo, objs);
    EXPECT_TRUE(has_code(result, "L2-002"));
}

// ── L2-003: track section references unknown boundary_node ───────────────────

TEST(Layer2Validator, OtReferencesUnknownBoundaryNode_ReportsL2003)
{
    const uint64_t kSec = infra_uid(TRACK_SECTION, 1, 1);
    const uint64_t kGhostBnd = infra_uid(BOUNDARY_NODE, 1, 99);  // not in boundary_nodes

    json topo = {{"boundary_nodes", json::array()},  // empty — kGhostBnd not defined
                 {"track_sections",
                  {{{"uid", kSec},
                    {"pID", "t1"},
                    {"sideA", {{"neighborUID", kGhostBnd}}},
                    {"sideB", {{"neighborUID", kGhostBnd}}}}}},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer2Validator v;
    const auto result = v.validate(1, topo, objs);
    EXPECT_TRUE(has_code(result, "L2-003"));
}

// ── L2-004: switch leg references unknown track section ───────────────────────

TEST(Layer2Validator, SwitchLegReferencesUnknownSection_ReportsL2004)
{
    const uint64_t kSw = infra_uid(SWITCH_KIND, 1, 1);
    const uint64_t kGhostSec = infra_uid(TRACK_SECTION, 1, 99);  // not in track_sections

    json topo = {
        {"boundary_nodes", json::array()},
        {"track_sections", json::array()},
        {"switches", {{{"uid", kSw}, {"pID", "p1"}, {"trunk", {{"neighborUID", kGhostSec}}}}}}};
    json objs = {{"signals", json::array()}};

    Layer2Validator v;
    const auto result = v.validate(1, topo, objs);
    EXPECT_TRUE(has_code(result, "L2-004"));
}

// ── L2-005: signal governs unknown track section ─────────────────────────────

TEST(Layer2Validator, SignalGovernsMissingSection_ReportsL2005)
{
    const uint64_t kSig = infra_uid(SIGNAL_KIND, 1, 1);
    const uint64_t kGhostSec = infra_uid(TRACK_SECTION, 1, 99);

    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections", json::array()},
                 {"switches", json::array()}};
    json objs = {{"signals",
                  {{{"uid", kSig},
                    {"pID", "p1"},
                    {"governs_section", kGhostSec},
                    {"initial_aspect", "STOP"}}}}};

    Layer2Validator v;
    const auto result = v.validate(1, topo, objs);
    EXPECT_TRUE(has_code(result, "L2-005"));
}
