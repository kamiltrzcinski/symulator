#include "services/uid_generator_service.hpp"

#include <limits>

#include "registry/uid_validator.hpp"

namespace symulator::tools
{

UidExhaustedException::UidExhaustedException()
    : std::runtime_error("No available INSTANCE in this SCOPE - all 65534 slots are taken")
{
}

UidGeneratorService::UidGeneratorService(const UidValidator& validator) noexcept
    : validator_(validator)
{
}

UID UidGeneratorService::generate(UIDDomain domain, UIDKind kind, std::uint16_t scope,
                                  std::uint16_t first_instance) const
{
    if (first_instance == 0)
    {
        throw std::invalid_argument("INSTANCE must be in range 0x0001-0xFFFF");
    }

    constexpr std::uint32_t instance_count = std::numeric_limits<std::uint16_t>::max();
    const std::uint32_t start = first_instance;

    for (std::uint32_t offset = 0; offset < instance_count; ++offset)
    {
        const auto instance =
            static_cast<std::uint16_t>(((start - 1U + offset) % instance_count) + 1U);
        const UID candidate = make_uid(domain, kind, scope, instance);
        if (uid_is_safe_json_integer(candidate) && validator_.isAvailable(candidate))
        {
            return candidate;
        }
    }

    throw UidExhaustedException();
}

}  // namespace symulator::tools
