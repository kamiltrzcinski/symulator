#include <gtest/gtest.h>
#include <engine/core/types.hpp>

#include <tests/common/param_test_helpers.hpp>

#include <array>
#include <cstdint>
using namespace engine::core;

namespace
{

struct UidRoundtripCase
{
    const char* name;
    UIDDomain domain;
    UIDKind kind;
    std::uint16_t scope;
    std::uint16_t instance;
};

class UidRoundtripTest : public ::testing::TestWithParam<UidRoundtripCase>
{
};

TEST_P(UidRoundtripTest, EncodeDecodeRoundtrip)
{
    const auto param = GetParam();
    const auto uid = make_uid(param.domain, param.kind, param.scope, param.instance);

    EXPECT_EQ(uid_domain(uid), param.domain);
    EXPECT_EQ(uid_kind(uid), param.kind);
    EXPECT_EQ(uid_scope(uid), param.scope);
    EXPECT_EQ(uid_instance(uid), param.instance);
}

const std::array<UidRoundtripCase, 19> kUidRoundtripCases{{
    {"RollingStockCarrier", UIDDomain::ROLLING_STOCK, UIDKind::CARRIER, 0, 1},
    {"RollingStockVehicleType", UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE_TYPE, 0x01B3, 42},
    {"RollingStockVehicle", UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0x0001, 1000},
    {"RollingStockTrainConsist", UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST, 0, 7},
    {"InfraStation", UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1},
    {"InfraTrackSection", UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 202},
    {"InfraSwitch", UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 2, 5},
    {"InfraSignal", UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1},
    {"InfraDerailer", UIDDomain::INFRASTRUCTURE, UIDKind::DERAILER, 3, 2},
    {"InfraBlockSection", UIDDomain::INFRASTRUCTURE, UIDKind::BLOCK_SECTION, 1, 3},
    {"InfraBoundaryNode", UIDDomain::INFRASTRUCTURE, UIDKind::BOUNDARY_NODE, 1, 1},
    {"InfraLevelCrossing", UIDDomain::INFRASTRUCTURE, UIDKind::LEVEL_CROSSING, 2, 1},
    {"InfraAxleCounter", UIDDomain::INFRASTRUCTURE, UIDKind::AXLE_COUNTER, 1, 4},
    {"InfraInterlocking", UIDDomain::INFRASTRUCTURE, UIDKind::INTERLOCKING, 3, 1},
    {"InfraPowerSupply", UIDDomain::INFRASTRUCTURE, UIDKind::POWER_SUPPLY, 1, 1},
    {"InfraDispatchArea", UIDDomain::INFRASTRUCTURE, UIDKind::DISPATCH_AREA, 3, 1},
    {"OpsRoute", UIDDomain::OPERATIONS, UIDKind::ROUTE, 1, 5},
    {"OpsAlarm", UIDDomain::OPERATIONS, UIDKind::ALARM, 0, 1},
    {"OpsDispatchExchange", UIDDomain::OPERATIONS, UIDKind::DISPATCH_EXCHANGE, 2, 3},
}};

INSTANTIATE_TEST_SUITE_P(UidRoundtripCases, UidRoundtripTest,
                         ::testing::ValuesIn(kUidRoundtripCases),
                         tests::common::param_name<UidRoundtripCase>);

struct UidBoundaryCase
{
    const char* name;
    UIDDomain domain;
    UIDKind kind;
    std::uint16_t scope;
    std::uint16_t instance;
};

class UidBoundaryTest : public ::testing::TestWithParam<UidBoundaryCase>
{
};

TEST_P(UidBoundaryTest, PreservesScopeInstanceAndJsonSafety)
{
    const auto param = GetParam();
    const auto uid = make_uid(param.domain, param.kind, param.scope, param.instance);

    EXPECT_EQ(uid_scope(uid), param.scope);
    EXPECT_EQ(uid_instance(uid), param.instance);
    EXPECT_TRUE(uid_is_safe_json_integer(uid));
}

const std::array<UidBoundaryCase, 2> kUidBoundaryCases{{
    {"MaxScopeAndInstance", UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 0xFFFF, 0xFFFF},
    {"MaxDomainAndKind", UIDDomain::OPERATIONS, UIDKind::DISPATCH_EXCHANGE, 0xFFFF, 0xFFFF},
}};

INSTANTIATE_TEST_SUITE_P(UidBoundaryCases, UidBoundaryTest, ::testing::ValuesIn(kUidBoundaryCases),
                         tests::common::param_name<UidBoundaryCase>);

// ── JSON safety ───────────────────────────────────────────────────────────────

TEST(UidCodec, AllKindsAreSafeJsonIntegers)
{
    // Max 48-bit UID: domain=0xFF, kind=0xFF, scope=0xFFFF, instance=0xFFFF
    // Bits 47-0 all set = 2^48 - 1 = 0x0000_FFFF_FFFF_FFFF < 2^53
    constexpr std::uint64_t max48 = 0x0000'FFFF'FFFF'FFFFull;
    EXPECT_LT(max48, UID_MAX_SAFE_JSON_INTEGER + 1);

    for (const auto& param : kUidRoundtripCases)
    {
        auto uid = make_uid(param.domain, param.kind, 0xFFFF, 0xFFFF);
        EXPECT_TRUE(uid_is_safe_json_integer(uid))
            << "Not safe JSON integer for domain=" << static_cast<int>(param.domain)
            << " kind=" << static_cast<int>(param.kind);
    }
}

struct UidCollisionCase
{
    const char* name;
    UIDDomain domain_a;
    UIDKind kind_a;
    std::uint16_t scope_a;
    std::uint16_t instance_a;
    UIDDomain domain_b;
    UIDKind kind_b;
    std::uint16_t scope_b;
    std::uint16_t instance_b;
};

class UidCollisionTest : public ::testing::TestWithParam<UidCollisionCase>
{
};

TEST_P(UidCollisionTest, DistinctInputsDoNotCollide)
{
    const auto param = GetParam();
    const auto a = make_uid(param.domain_a, param.kind_a, param.scope_a, param.instance_a);
    const auto b = make_uid(param.domain_b, param.kind_b, param.scope_b, param.instance_b);
    EXPECT_NE(a.value, b.value);
}

const std::array<UidCollisionCase, 5> kUidCollisionCases{{
    {"DifferentKindsTrackVsSignal", UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1,
     UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1},
    {"DifferentKindsTrackVsSwitch", UIDDomain::INFRASTRUCTURE, UIDKind::TRACK_SECTION, 1, 1,
     UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 1},
    {"DifferentScopes", UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1, UIDDomain::INFRASTRUCTURE,
     UIDKind::SIGNAL, 2, 1},
    {"DifferentInstances", UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1,
     UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 2},
    {"DifferentDomains", UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE_TYPE, 1, 1,
     UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1},
}};

INSTANTIATE_TEST_SUITE_P(UidCollisionCases, UidCollisionTest,
                         ::testing::ValuesIn(kUidCollisionCases),
                         tests::common::param_name<UidCollisionCase>);

struct UidHasKindCase
{
    const char* name;
    UID uid;
    UIDDomain expected_domain;
    UIDKind expected_kind;
    bool expected_match;
};

class UidHasKindTest : public ::testing::TestWithParam<UidHasKindCase>
{
};

TEST_P(UidHasKindTest, MatchesExpectedDomainAndKind)
{
    const auto param = GetParam();
    EXPECT_EQ(uid_has_kind(param.uid, param.expected_domain, param.expected_kind),
              param.expected_match);
}

const std::array<UidHasKindCase, 3> kUidHasKindCases{{
    {"TrueMatch", make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1),
     UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, true},
    {"FalseWrongDomain", make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 1),
     UIDDomain::INFRASTRUCTURE, UIDKind::VEHICLE, false},
    {"FalseWrongKind", make_uid(UIDDomain::INFRASTRUCTURE, UIDKind::SWITCH, 1, 1),
     UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, false},
}};

INSTANTIATE_TEST_SUITE_P(UidHasKindCases, UidHasKindTest, ::testing::ValuesIn(kUidHasKindCases),
                         tests::common::param_name<UidHasKindCase>);

struct UidKnownValueCase
{
    const char* name;
    UIDDomain domain;
    UIDKind kind;
    std::uint16_t scope;
    std::uint16_t instance;
    std::uint64_t expected_value;
};

class UidKnownValueTest : public ::testing::TestWithParam<UidKnownValueCase>
{
};

TEST_P(UidKnownValueTest, ProducesStableRegressionValue)
{
    const auto param = GetParam();
    const auto uid = make_uid(param.domain, param.kind, param.scope, param.instance);
    EXPECT_EQ(uid.value, param.expected_value);
}

const std::array<UidKnownValueCase, 3> kUidKnownValueCases{{
    {"CarrierInstance1", UIDDomain::ROLLING_STOCK, UIDKind::CARRIER, 0, 1,
     0x0000'0104'0000'0001ULL},
    {"StationGOr", UIDDomain::INFRASTRUCTURE, UIDKind::STATION, 1, 1, 0x0000'0211'0001'0001ULL},
    {"SignalGOrInstance1", UIDDomain::INFRASTRUCTURE, UIDKind::SIGNAL, 1, 1,
     0x0000'0215'0001'0001ULL},
}};

INSTANTIATE_TEST_SUITE_P(UidKnownValueCases, UidKnownValueTest,
                         ::testing::ValuesIn(kUidKnownValueCases),
                         tests::common::param_name<UidKnownValueCase>);

}  // namespace