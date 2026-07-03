// server/include/server/terminal/trains_command.hpp
//
// `trains` — list all active trains.  Reads the last published AtomicSnapshot
// (never the live EngineState), so it is race-free by construction.

#pragma once

#include "server/terminal/terminal_command.hpp"

#include "engine/core/engine_snapshot.hpp"

namespace server::terminal
{

class TrainsCommand final : public ITerminalCommand
{
public:
    explicit TrainsCommand(const engine::core::AtomicSnapshot& snapshot);

    std::string_view name() const override { return "trains"; }
    std::string_view help() const override { return "trains — list active trains"; }
    Permission required_permission() const override { return Permission::LOGS; }

    std::string execute(const std::vector<std::string>& args) override;

private:
    const engine::core::AtomicSnapshot& snapshot_;
};

}  // namespace server::terminal
