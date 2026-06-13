#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "domain/uid_types.hpp"

namespace symulator::tools
{

struct UidLegendEntry
{
    UIDDomain domain;
    std::string_view domain_name;
    UIDKind kind;
    std::string_view kind_name;
    std::string_view scope_semantics;
};

inline constexpr std::array<UIDDomain, 3> kKnownUidDomains{
    UIDDomain::ROLLING_STOCK,
    UIDDomain::INFRASTRUCTURE,
    UIDDomain::OPERATIONS,
};

inline constexpr std::array<UIDKind, 19> kKnownUidKinds{
    UIDKind::VEHICLE_TYPE,      UIDKind::VEHICLE,         UIDKind::TRAIN_CONSIST,
    UIDKind::CARRIER,           UIDKind::STATION,         UIDKind::DISPATCH_AREA,
    UIDKind::TRACK_SECTION,     UIDKind::SWITCH,          UIDKind::SIGNAL,
    UIDKind::DERAILER,          UIDKind::BLOCK_SECTION,   UIDKind::BOUNDARY_NODE,
    UIDKind::LEVEL_CROSSING,    UIDKind::AXLE_COUNTER,    UIDKind::INTERLOCKING,
    UIDKind::POWER_SUPPLY,      UIDKind::ROUTE,           UIDKind::ALARM,
    UIDKind::DISPATCH_EXCHANGE,
};

inline constexpr std::array<UidLegendEntry, 19> kUidLegendEntries{{
    {UIDDomain::ROLLING_STOCK, "ROLLING_STOCK", UIDKind::VEHICLE_TYPE, "VEHICLE_TYPE",
     "Vehicle series/family code, or 0"},
    {UIDDomain::ROLLING_STOCK, "ROLLING_STOCK", UIDKind::VEHICLE, "VEHICLE",
     "Vehicle series/family code, or 0"},
    {UIDDomain::ROLLING_STOCK, "ROLLING_STOCK", UIDKind::TRAIN_CONSIST, "TRAIN_CONSIST",
     "Vehicle series/family code, or 0"},
    {UIDDomain::ROLLING_STOCK, "ROLLING_STOCK", UIDKind::CARRIER, "CARRIER",
     "Always 0"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::STATION, "STATION",
     "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::DISPATCH_AREA,
     "DISPATCH_AREA", "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::TRACK_SECTION,
     "TRACK_SECTION", "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::SWITCH, "SWITCH",
     "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::SIGNAL, "SIGNAL",
     "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::DERAILER, "DERAILER",
     "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::BLOCK_SECTION, "BLOCK_SECTION",
     "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::BOUNDARY_NODE, "BOUNDARY_NODE",
     "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::LEVEL_CROSSING,
     "LEVEL_CROSSING", "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::AXLE_COUNTER, "AXLE_COUNTER",
     "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::INTERLOCKING, "INTERLOCKING",
     "Station instance number"},
    {UIDDomain::INFRASTRUCTURE, "INFRASTRUCTURE", UIDKind::POWER_SUPPLY, "POWER_SUPPLY",
     "Station instance number"},
    {UIDDomain::OPERATIONS, "OPERATIONS", UIDKind::ROUTE, "ROUTE",
     "Station instance number, or 0 for session-global"},
    {UIDDomain::OPERATIONS, "OPERATIONS", UIDKind::ALARM, "ALARM",
     "Station instance number, or 0 for session-global"},
    {UIDDomain::OPERATIONS, "OPERATIONS", UIDKind::DISPATCH_EXCHANGE,
     "DISPATCH_EXCHANGE", "Station instance number, or 0 for session-global"},
}};

consteval bool uidLegendIsComplete()
{
    if (kUidLegendEntries.size() != kKnownUidKinds.size())
    {
        return false;
    }

    for (const UIDDomain domain : kKnownUidDomains)
    {
        bool found = false;
        for (const auto& entry : kUidLegendEntries)
        {
            found = found || entry.domain == domain;
        }
        if (!found)
        {
            return false;
        }
    }

    for (const UIDKind kind : kKnownUidKinds)
    {
        std::size_t matches = 0;
        for (const auto& entry : kUidLegendEntries)
        {
            if (entry.kind == kind)
            {
                ++matches;
            }
        }
        if (matches != 1)
        {
            return false;
        }
    }

    return true;
}

static_assert(uidLegendIsComplete(),
              "UID legend must contain every known UIDDomain and UIDKind exactly once");

[[nodiscard]] constexpr std::uint8_t uidDomainValue(const UidLegendEntry& entry) noexcept
{
    return static_cast<std::uint8_t>(entry.domain);
}

[[nodiscard]] constexpr std::uint8_t uidKindValue(const UidLegendEntry& entry) noexcept
{
    return static_cast<std::uint8_t>(entry.kind);
}

}  // namespace symulator::tools
