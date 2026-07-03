// server/include/server/terminal/terminal_session.hpp
//
// One interactive terminal session: authentication state + command dispatch.
// process_line() is pure string→string (no I/O), so the whole login and
// permission logic is unit-testable without any transport.  Transports
// (StdinTerminal today; telnet/admin-websocket later) feed lines in and print
// the returned text.
//
// Built-in verbs (always available, not part of the registry):
//   login <user> <password>   authenticate against the IUserStore
//   logout                     drop the authenticated user
//   help                       list built-ins + registry commands the current
//                              user is allowed to run

#pragma once

#include "server/terminal/command_registry.hpp"
#include "server/terminal/user_store.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace server::terminal
{

class TerminalSession
{
public:
    TerminalSession(const CommandRegistry& registry, const IUserStore& users);

    /// Process one input line and return the text to display.
    std::string process_line(std::string_view line);

    bool is_authenticated() const noexcept { return user_.has_value(); }

private:
    std::string handle_login(const std::vector<std::string>& args);
    std::string build_help() const;

    const CommandRegistry& registry_;
    const IUserStore& users_;
    std::optional<UserAccount> user_;
};

}  // namespace server::terminal
