// server/include/server/ownership_guard.hpp
// Thread-safe registry that tracks which PlayerID currently controls each
// dispatch area UID.
//
// Threading model: all public methods are safe to call from any thread.

#pragma once

#include "engine/core/types.hpp"

#include <mutex>
#include <optional>
#include <unordered_map>

namespace server
{

class OwnershipGuard
{
public:
    OwnershipGuard() = default;

    // Assign ownership of a dispatch area to a player.
    // Overwrites any existing owner for that dispatch area.
    void set_owner(engine::core::UID dispatch_area, const engine::core::PlayerID& player);

    // Release ownership of a dispatch area (no-op if nobody owns it).
    void release(engine::core::UID dispatch_area);

    // Release all dispatch areas currently owned by the given player.
    void release_all(const engine::core::PlayerID& player);

    // Returns true iff the given player is the current owner of the dispatch area.
    bool check(engine::core::UID dispatch_area, const engine::core::PlayerID& player) const;

    // Returns the current owner of a dispatch area, or nullopt if unowned.
    std::optional<engine::core::PlayerID> get_owner(engine::core::UID dispatch_area) const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<engine::core::UID, engine::core::PlayerID, std::hash<engine::core::UID>>
        owners_;
};

}  // namespace server
