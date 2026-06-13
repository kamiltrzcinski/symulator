#include "registry/uid_registry.hpp"

#include <utility>

namespace symulator::tools
{

bool UidRegistry::insert(UID uid, std::filesystem::path source_file)
{
    return entries_.emplace(uid, std::move(source_file)).second;
}

bool UidRegistry::contains(UID uid) const noexcept
{
    return entries_.contains(uid);
}

std::optional<std::filesystem::path> UidRegistry::sourceFile(UID uid) const
{
    const auto it = entries_.find(uid);
    if (it == entries_.end())
    {
        return std::nullopt;
    }
    return it->second;
}

std::vector<UidRegistryEntry> UidRegistry::entries() const
{
    std::vector<UidRegistryEntry> result;
    result.reserve(entries_.size());
    for (const auto& [uid, source_file] : entries_)
    {
        result.push_back({uid, source_file});
    }
    return result;
}

std::size_t UidRegistry::size() const noexcept
{
    return entries_.size();
}

bool UidRegistry::empty() const noexcept
{
    return entries_.empty();
}

void UidRegistry::clear() noexcept
{
    entries_.clear();
}

}  // namespace symulator::tools
