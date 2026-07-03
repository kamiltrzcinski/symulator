// server/src/terminal/command_registry.cpp

#include "server/terminal/command_registry.hpp"

#include <stdexcept>
#include <utility>

namespace server::terminal
{

void CommandRegistry::add(std::unique_ptr<ITerminalCommand> cmd)
{
    std::string name{cmd->name()};
    auto [it, inserted] = commands_.emplace(std::move(name), std::move(cmd));
    if (!inserted)
        throw std::invalid_argument("CommandRegistry: duplicate command name '" + it->first + "'");
}

ITerminalCommand* CommandRegistry::find(std::string_view name) const
{
    auto it = commands_.find(name);
    return (it == commands_.end()) ? nullptr : it->second.get();
}

void CommandRegistry::for_each(const std::function<void(const ITerminalCommand&)>& fn) const
{
    for (const auto& [name, cmd] : commands_)
        fn(*cmd);
}

}  // namespace server::terminal
