// engine/include/engine/core/fleet_command.hpp
// Runtime fleet mutations, safe to request from any thread via
// EngineLoop::enqueue_fleet_command().  The ENGINE thread drains the queue at
// the start of each tick, re-validates against the current world state, and
// applies the mutation through TrainFleet.
//
// This is the seam the future TrainScheduler (plan_engine.md E5) plugs into —
// the server terminal's spawn/despawn commands are simply its first client.

#pragma once

#include "engine/core/types.hpp"
#include "engine/sim/train_sim.hpp"

#include <variant>

namespace engine::core
{

struct SpawnRequest
{
    sim::TrainSimState initial;
    UID from_uid;  ///< UID of the boundary node (or section) behind the train.
};

struct DespawnRequest
{
    UID train_uid;
};

using FleetCommand = std::variant<SpawnRequest, DespawnRequest>;

}  // namespace engine::core
