// server/src/terminal/terminal_command.cpp

#include "server/terminal/terminal_command.hpp"

namespace server::terminal
{

const char* to_string(Permission p)
{
    switch (p)
    {
        case Permission::SPAWN:
            return "SPAWN";
        case Permission::DESPAWN:
            return "DESPAWN";
        case Permission::KICK:
            return "KICK";
        case Permission::BAN:
            return "BAN";
        case Permission::RESET:
            return "RESET";
        case Permission::SETTINGS:
            return "SETTINGS";
        case Permission::LOGS:
            return "LOGS";
    }
    return "UNKNOWN";
}

std::set<Permission> all_permissions()
{
    return {Permission::SPAWN, Permission::DESPAWN,  Permission::KICK, Permission::BAN,
            Permission::RESET, Permission::SETTINGS, Permission::LOGS};
}

}  // namespace server::terminal
