// server/include/server/terminal/command_registry.hpp
//
// Open registry of terminal commands.  Adding a new command to the server is
// one `registry.add(std::make_unique<MyCommand>(...))` line — no dispatch code
// changes anywhere else.

#pragma once

#include "server/terminal/terminal_command.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace server::terminal
{

class CommandRegistry
{
public:
    /// Register a command.  Throws std::invalid_argument on a duplicate name.
    void add(std::unique_ptr<ITerminalCommand> cmd);

    /// Look up a command by its name.  Returns nullptr when unknown.
    ITerminalCommand* find(std::string_view name) const;

    /// Visit all commands in name order (used to build `help`).
    void for_each(const std::function<void(const ITerminalCommand&)>& fn) const;

private:
    // std::less<> enables heterogeneous string_view lookup.
    std::map<std::string, std::unique_ptr<ITerminalCommand>, std::less<>> commands_;
};

}  // namespace server::terminal
