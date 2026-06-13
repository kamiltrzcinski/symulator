#include "registry/uid_validator.hpp"

#include "registry/uid_registry.hpp"

namespace symulator::tools
{

UidValidator::UidValidator(const UidRegistry& registry) noexcept
    : registry_(registry)
{
}

bool UidValidator::isAvailable(UID uid) const noexcept
{
    return !registry_.contains(uid);
}

}  // namespace symulator::tools
