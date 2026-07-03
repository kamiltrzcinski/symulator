// server/include/server/terminal/spawn_command.hpp
//
// `spawn <consist uid|pID> <boundary uid|pID>` — spawn a train from the fleet
// at a boundary node.  Validates synchronously what it can (consist exists,
// boundary exists, adjacent section free *right now*, train not already
// active) and enqueues a SpawnRequest; the ENGINE thread re-validates when it
// drains the queue.

#pragma once

#include "server/terminal/terminal_command.hpp"

#include "engine/core/engine_snapshot.hpp"
#include "engine/core/fleet_command.hpp"
#include "engine/core/fleet_registry.hpp"

#include <functional>

namespace server::terminal
{

class SpawnCommand final : public ITerminalCommand
{
public:
    using EnqueueFn = std::function<void(engine::core::FleetCommand)>;

    SpawnCommand(const engine::core::FleetRegistry& fleet,
                 const engine::core::AtomicSnapshot& snapshot, EnqueueFn enqueue);

    std::string_view name() const override { return "spawn"; }
    std::string_view help() const override
    {
        return "spawn <consist uid|pID> <boundary uid|pID> — spawn a train at a boundary node";
    }
    Permission required_permission() const override { return Permission::SPAWN; }

    std::string execute(const std::vector<std::string>& args) override;

private:
    const engine::core::FleetRegistry& fleet_;
    const engine::core::AtomicSnapshot& snapshot_;
    EnqueueFn enqueue_;
};

}  // namespace server::terminal
