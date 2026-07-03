// server/src/terminal/spawn_command.cpp

#include "server/terminal/spawn_command.hpp"
#include "server/terminal/lookup.hpp"

#include "engine/core/spawn_resolver.hpp"
#include "engine/sim/train_sim.hpp"

#include <exception>
#include <utility>
#include <variant>
#include <vector>

namespace server::terminal
{

SpawnCommand::SpawnCommand(const engine::core::FleetRegistry& fleet,
                           const engine::core::AtomicSnapshot& snapshot, EnqueueFn enqueue)
    : fleet_(fleet), snapshot_(snapshot), enqueue_(std::move(enqueue))
{
}

std::string SpawnCommand::execute(const std::vector<std::string>& args)
{
    if (args.size() != 2)
        return "usage: spawn <consist uid|pID> <boundary uid|pID>";

    const auto snap = snapshot_.load();
    if (!snap)
        return "world snapshot not available yet";

    const auto consist_uid = resolve_consist_uid(fleet_, args[0]);
    if (!consist_uid)
        return "unknown consist: " + args[0];
    const auto& consist = fleet_.get_consist(*consist_uid);

    for (const auto& train : snap->trains)
    {
        if (train.uid == *consist_uid)
            return "train already active: " + consist.pid;
    }

    const auto boundary_uid = resolve_boundary_uid(*snap, args[1]);
    if (!boundary_uid)
        return "unknown boundary node: " + args[1];

    const auto resolved = engine::core::resolve_spawn_at_boundary(*snap, *boundary_uid);
    if (const auto* error = std::get_if<engine::core::SpawnError>(&resolved))
        return std::string{"spawn rejected: "} + engine::core::to_string(*error);
    const auto& point = std::get<engine::core::SpawnPoint>(resolved);

    std::vector<engine::core::Vehicle> vehicles;
    vehicles.reserve(consist.vehicle_uids.size());
    for (const auto& vehicle_uid : consist.vehicle_uids)
        vehicles.push_back(fleet_.get_vehicle(vehicle_uid));

    engine::sim::TrainSimState initial;
    try
    {
        initial = engine::sim::make_train_sim_state(consist, vehicles, point.section_uid);
    }
    catch (const std::exception& e)
    {
        return std::string{"spawn rejected: "} + e.what();
    }

    enqueue_(engine::core::SpawnRequest{std::move(initial), point.from_uid});
    return "spawn queued: " + consist.pid + " (uid " + std::to_string(consist_uid->value) +
           ") at section uid " + std::to_string(point.section_uid.value);
}

}  // namespace server::terminal
