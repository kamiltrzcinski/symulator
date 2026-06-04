#pragma once

#include <engine/core/types.hpp>

namespace tests::common::fleet
{

using engine::core::UID;

// VehicleType UIDs: make_uid(RS, VEHICLE_TYPE, 0, N)
inline constexpr UID VT_201E = UID{0x010100000001ULL};
inline constexpr UID VT_452W = UID{0x010100000002ULL};
inline constexpr UID VT_EU07 = UID{0x010100000003ULL};
inline constexpr UID VT_EN57 = UID{0x010100000004ULL};
inline constexpr UID VT_SM42 = UID{0x010100000005ULL};

// Vehicle UIDs: make_uid(RS, VEHICLE, 0, N)
inline constexpr UID VEH_ET22_001 = UID{0x010200000001ULL};
inline constexpr UID VEH_452W_537_0001 = UID{0x010200000002ULL};
inline constexpr UID VEH_452W_537_0002 = UID{0x010200000003ULL};
inline constexpr UID VEH_EU07_001 = UID{0x010200000004ULL};
inline constexpr UID VEH_EU07_002 = UID{0x010200000005ULL};
inline constexpr UID VEH_EN57_001 = UID{0x010200000006ULL};
inline constexpr UID VEH_ET22_002 = UID{0x010200000007ULL};
inline constexpr UID VEH_SM42_001 = UID{0x010200000008ULL};

// TrainConsist UIDs: make_uid(RS, TRAIN_CONSIST, 0, N)
inline constexpr UID TRN_TOW543210 = UID{0x010300000001ULL};
inline constexpr UID TRN_TEST = UID{0x010300000002ULL};
inline constexpr UID TRN_TEST_CARRIER = UID{0x010300000003ULL};
inline constexpr UID TRN_NULL_CARRIER = UID{0x010300000004ULL};
inline constexpr UID TRN_MISS_CARRIER = UID{0x010300000005ULL};
inline constexpr UID TRN_BAD_CARRIER = UID{0x010300000006ULL};

}  // namespace tests::common::fleet
