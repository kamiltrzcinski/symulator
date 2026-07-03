// server/src/terminal/despawn_command.cpp

#include "server/terminal/despawn_command.hpp"
#include "server/terminal/lookup.hpp"

#include <algorithm>
#include <utility>

namespace server::terminal
{

DespawnCommand::DespawnCommand(const engine::core::FleetRegistry& fleet,
                               const engine::core::AtomicSnapshot& snapshot, EnqueueFn enqueue)
    : fleet_(fleet), snapshot_(snapshot), enqueue_(std::move(enqueue))
{
}

std::string DespawnCommand::execute(const std::vector<std::string>& args)
{
    if (args.size() != 1)
        return "usage: despawn <train uid|consist pID>";

    const auto snap = snapshot_.load();
    if (!snap)
        return "world snapshot not available yet";

    // A train's UID is its consist's UID, so the argument resolves the same way
    // as for spawn: decimal UID first, consist pID as fallback.
    auto train_uid = resolve_consist_uid(fleet_, args[0]);
    if (!train_uid)
    {
        if (auto value = parse_uint(args[0]))
            train_uid = engine::core::UID{*value};
        else
            return "unknown train: " + args[0];
    }

    const bool active = std::any_of(snap->trains.begin(), snap->trains.end(),
                                    [&](const auto& train) { return train.uid == *train_uid; });
    if (!active)
        return "no active train: " + args[0];

    enqueue_(engine::core::DespawnRequest{*train_uid});
    return "despawn queued: uid " + std::to_string(train_uid->value);
}

}  // namespace server::terminal
