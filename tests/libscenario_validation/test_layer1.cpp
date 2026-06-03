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

TEST_F(Layer1GdyniaOrlowo, ValidScenario_NoDuplicateUIDs)
{
    Layer1Validator v;
    const auto result = v.validate(topo_, objs_);
    EXPECT_FALSE(has_code(result, "L1-002"));
}

// ── L1-001: missing required field ───────────────────────────────────────────

TEST(Layer1Validator, MissingUID_ReportsL1001)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {
                      {{"pID", "abc"},  // uid absent
                       {"sideA", {{"neighborUID", 1000}}},
                       {"sideB", {{"neighborUID", 1001}}}},
                  }},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-001"));
}

TEST(Layer1Validator, MissingPID_ReportsL1001)
{
    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {
                      {{"uid", 1234567890ULL},  // pID absent
                       {"sideA", {{"neighborUID", 1000}}},
                       {"sideB", {{"neighborUID", 1001}}}},
                  }},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-001"));
}

// ── L1-002: duplicate UID ─────────────────────────────────────────────────────

TEST(Layer1Validator, DuplicateUID_ReportsL1002)
{
    const uint64_t kDup = 578721382375425ULL;  // some valid INFRA UID
    json topo = {{"boundary_nodes",
                  {{{"uid", kDup}, {"pID", "p1"}}, {{"uid", kDup}, {"pID", "p2"}}}},  // duplicate!
                 {"track_sections", json::array()},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-002"));
}

// ── L1-003: unknown izUID ────────────────────────────────────────────────────

TEST(Layer1Validator, UnknownIzUID_ReportsL1003)
{
    const uint64_t kSec = 578721382572049ULL;
    const uint64_t kBnd = 578721382375425ULL;
    const uint64_t kUnknownIz = 999999ULL;  // not defined by any switch

    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {{{"uid", kSec},
                    {"pID", "px"},
                    {"sideA", {{"neighborUID", kBnd}}},
                    {"sideB", {{"neighborUID", 0}, {"izUID", kUnknownIz}}}}}},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-003"));
}

// ── L1-004: signalUIDs[] references unknown signal ────────────────────────────

TEST(Layer1Validator, TrackSectionReferencesUnknownSignal_ReportsL1004)
{
    const uint64_t kSec = 578721382572049ULL;
    const uint64_t kBnd = 578721382375425ULL;
    const uint64_t kGhostSig = 555555555555ULL;

    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {{{"uid", kSec},
                    {"pID", "t1"},
                    {"sideA", {{"neighborUID", kBnd}, {"signalUIDs", {kGhostSig}}}},
                    {"sideB", {{"neighborUID", kBnd}}}}}},
                 {"switches", json::array()}};
    json objs = {{"signals", json::array()}};  // ghost signal not defined

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-004"));
}

// ── L1-005: signal governs unknown section ────────────────────────────────────

TEST(Layer1Validator, SignalGovernsMissingSection_ReportsL1005)
{
    const uint64_t kSig = 580420923064321ULL;
    const uint64_t kGhostSec = 999999999999ULL;

    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections", json::array()},
                 {"switches", json::array()}};
    json objs = {{"signals",
                  {{{"uid", kSig},
                    {"pID", "p1"},
                    {"governs_section", kGhostSec},
                    {"initial_aspect", "STOP"}}}}};

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-005"));
}

// ── L1-006: invalid initial_aspect ──────────────────────────────────────────

TEST(Layer1Validator, InvalidAspect_ReportsL1006)
{
    const uint64_t kSec = 578721382572049ULL;
    const uint64_t kBnd = 578721382375425ULL;
    const uint64_t kSig = 580420923064321ULL;

    json topo = {{"boundary_nodes", json::array()},
                 {"track_sections",
                  {{{"uid", kSec},
                    {"pID", "t1"},
                    {"sideA", {{"neighborUID", kBnd}}},
                    {"sideB", {{"neighborUID", kBnd}}}}}},
                 {"switches", json::array()}};
    json objs = {{"signals",
                  {{{"uid", kSig},
                    {"pID", "p1"},
                    {"governs_section", kSec},
                    {"initial_aspect", "SUPER_GREEN"}}}}};  // invalid!

    Layer1Validator v;
    const auto result = v.validate(topo, objs);
    EXPECT_TRUE(has_code(result, "L1-006"));
}
