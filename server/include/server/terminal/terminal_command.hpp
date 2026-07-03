// server/include/server/terminal/terminal_command.hpp
//
// Built-in server terminal — command interface and permission model.
//
// Every terminal command is a separate class implementing ITerminalCommand and
// is registered in a CommandRegistry (one registration line per command).
// TerminalSession dispatches a parsed input line to a command only after
// checking that the logged-in user holds the command's required permission —
// this is how per-role capabilities (e.g. a moderator who may spawn/despawn
// and kick/ban but never reset or change settings) are enforced without any
// changes to the commands themselves.

#pragma once

#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace server::terminal
{

enum class Permission
{
    SPAWN,
    DESPAWN,
    KICK,
    BAN,
    RESET,
    SETTINGS,
    LOGS,
};

const char* to_string(Permission p);

/// Full permission set — the default single admin account holds all of these.
std::set<Permission> all_permissions();

class ITerminalCommand
{
public:
    virtual ~ITerminalCommand() = default;

    /// Command word typed by the user, e.g. "spawn".
    virtual std::string_view name() const = 0;

    /// One-line usage/description shown by `help`.
    virtual std::string_view help() const = 0;

    /// Permission the logged-in user must hold to run this command.
    virtual Permission required_permission() const = 0;

    /// Execute with the arguments following the command word.
    /// Returns the text to print on the terminal (may be multi-line).
    virtual std::string execute(const std::vector<std::string>& args) = 0;
};

}  // namespace server::terminal
