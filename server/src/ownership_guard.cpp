// server/src/ownership_guard.cpp

#include "server/ownership_guard.hpp"

namespace server
{

void OwnershipGuard::set_owner(engine::core::UID dispatch_area,
                               const engine::core::PlayerID& player)
{
    std::scoped_lock lock{mutex_};
    owners_[dispatch_area] = player;
}

void OwnershipGuard::release(engine::core::UID dispatch_area)
{
    std::scoped_lock lock{mutex_};
    owners_.erase(dispatch_area);
}

void OwnershipGuard::release_all(const engine::core::PlayerID& player)
{
    std::scoped_lock lock{mutex_};
    for (auto it = owners_.begin(); it != owners_.end();)
    {
        if (it->second == player)
            it = owners_.erase(it);
        else
            ++it;
    }
}

bool OwnershipGuard::check(engine::core::UID dispatch_area,
                           const engine::core::PlayerID& player) const
{
    std::scoped_lock lock{mutex_};
    const auto it = owners_.find(dispatch_area);
    return (it != owners_.end()) && (it->second == player);
}

std::optional<engine::core::PlayerID> OwnershipGuard::get_owner(
    engine::core::UID dispatch_area) const
{
    std::scoped_lock lock{mutex_};
    const auto it = owners_.find(dispatch_area);
    if (it == owners_.end())
        return std::nullopt;
    return it->second;
}

}  // namespace server
