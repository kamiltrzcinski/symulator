#pragma once

#include "domain/uid_types.hpp"

namespace symulator::tools
{

class UidRegistry;

class UidValidator
{
public:
    explicit UidValidator(const UidRegistry& registry) noexcept;

    [[nodiscard]] bool isAvailable(UID uid) const noexcept;

private:
    const UidRegistry& registry_;
};

}  // namespace symulator::tools
