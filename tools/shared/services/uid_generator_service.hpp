#pragma once

#include <cstdint>
#include <stdexcept>

#include "domain/uid_types.hpp"

namespace symulator::tools
{

class UidValidator;

class UidExhaustedException : public std::runtime_error
{
public:
    UidExhaustedException();
};

class UidGeneratorService
{
public:
    explicit UidGeneratorService(const UidValidator& validator) noexcept;

    [[nodiscard]] UID generate(UIDDomain domain, UIDKind kind, std::uint16_t scope,
                               std::uint16_t first_instance = 1) const;

private:
    const UidValidator& validator_;
};

}  // namespace symulator::tools
