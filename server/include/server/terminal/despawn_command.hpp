// server/include/server/terminal/despawn_command.hpp
//
// `despawn <train uid|consist pID>` — remove an active train from the world.
// The ENGINE thread frees the train's current section when it drains the
// request.

#pragma once

#include "server/terminal/terminal_command.hpp"

#include "engine/core/engine_snapshot.hpp"
#include "engine/core/fleet_command.hpp"
#include "engine/core/fleet_registry.hpp"

#include <functional>

namespace server::terminal
{

class DespawnCommand final : public ITerminalCommand
{
public:
    using EnqueueFn = std::function<void(engine::core::FleetCommand)>;

    DespawnCommand(const engine::core::FleetRegistry& fleet,
                   const engine::core::AtomicSnapshot& snapshot, EnqueueFn enqueue);

    std::string_view name() const override { return "despawn"; }
    std::string_view help() const override
    {
        return "despawn <train uid|consist pID> — remove an active train from the world";
    }
    Permission required_permission() const override { return Permission::DESPAWN; }

    std::string execute(const std::vector<std::string>& args) override;

private:
    const engine::core::FleetRegistry& fleet_;
    const engine::core::AtomicSnapshot& snapshot_;
    EnqueueFn enqueue_;
};

}  // namespace server::terminal
