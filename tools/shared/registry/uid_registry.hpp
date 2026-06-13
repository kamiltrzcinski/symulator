#pragma once

#include <cstddef>
#include <filesystem>
#include <map>
#include <optional>
#include <vector>

#include "domain/uid_types.hpp"

namespace symulator::tools
{

struct UidRegistryEntry
{
    UID uid;
    std::filesystem::path source_file;
};

class UidRegistry
{
public:
    [[nodiscard]] bool insert(UID uid, std::filesystem::path source_file);
    [[nodiscard]] bool contains(UID uid) const noexcept;
    [[nodiscard]] std::optional<std::filesystem::path> sourceFile(UID uid) const;
    [[nodiscard]] std::vector<UidRegistryEntry> entries() const;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    void clear() noexcept;

private:
    std::map<UID, std::filesystem::path> entries_;
};

}  // namespace symulator::tools
