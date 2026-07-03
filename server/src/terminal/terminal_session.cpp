// server/src/terminal/terminal_session.cpp

#include "server/terminal/terminal_session.hpp"

#include <sstream>
#include <vector>

namespace server::terminal
{

namespace
{

std::vector<std::string> tokenize(std::string_view line)
{
    std::vector<std::string> tokens;
    std::istringstream stream{std::string{line}};
    std::string token;
    while (stream >> token)
        tokens.push_back(std::move(token));
    return tokens;
}

}  // namespace

TerminalSession::TerminalSession(const CommandRegistry& registry, const IUserStore& users)
    : registry_(registry), users_(users)
{
}

std::string TerminalSession::process_line(std::string_view line)
{
    const auto tokens = tokenize(line);
    if (tokens.empty())
        return "";

    const std::string& verb = tokens.front();
    const std::vector<std::string> args(tokens.begin() + 1, tokens.end());

    // ── Built-ins ─────────────────────────────────────────────────────────────
    if (verb == "login")
        return handle_login(args);
    if (verb == "logout")
    {
        if (!user_)
            return "not logged in";
        const std::string login = user_->login;
        user_.reset();
        return "logged out: " + login;
    }
    if (verb == "help")
        return build_help();

    // ── Registry commands: authentication, then authorisation ────────────────
    if (!user_)
        return "not logged in — use: login <user> <password>";

    ITerminalCommand* cmd = registry_.find(verb);
    if (!cmd)
        return "unknown command: " + verb + " (try `help`)";

    const Permission required = cmd->required_permission();
    if (!user_->permissions.contains(required))
        return "permission denied: `" + verb + "` requires " + to_string(required);

    return cmd->execute(args);
}

std::string TerminalSession::handle_login(const std::vector<std::string>& args)
{
    if (args.size() != 2)
        return "usage: login <user> <password>";

    auto account = users_.authenticate(args[0], args[1]);
    if (!account)
        return "invalid credentials";

    user_ = std::move(*account);
    return "logged in as " + user_->login;
}

std::string TerminalSession::build_help() const
{
    std::ostringstream out;
    out << "built-in commands:\n"
        << "  login <user> <password>\n"
        << "  logout\n"
        << "  help";

    if (!user_)
    {
        out << "\nlog in to see available commands";
        return out.str();
    }

    registry_.for_each(
        [&](const ITerminalCommand& cmd)
        {
            if (user_->permissions.contains(cmd.required_permission()))
                out << "\n  " << cmd.help();
        });
    return out.str();
}

}  // namespace server::terminal
