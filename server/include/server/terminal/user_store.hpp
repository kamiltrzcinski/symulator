// server/include/server/terminal/user_store.hpp
//
// Terminal authentication.  IUserStore is the seam for the future DB-backed
// store: user records will live in PostgreSQL with a password *hash* (never
// plaintext) and authenticate() will compare hashes — a PgUserStore drop-in,
// zero changes in TerminalSession.  Today the only implementation is
// InMemoryUserStore holding the single built-in admin account.

#pragma once

#include "server/terminal/terminal_command.hpp"

#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace server::terminal
{

struct UserAccount
{
    std::string login;
    std::string password;  // plaintext for the local debug terminal only
    std::set<Permission> permissions;
};

class IUserStore
{
public:
    virtual ~IUserStore() = default;

    /// Returns the account when the credentials match, std::nullopt otherwise.
    virtual std::optional<UserAccount> authenticate(std::string_view login,
                                                    std::string_view password) const = 0;
};

class InMemoryUserStore final : public IUserStore
{
public:
    explicit InMemoryUserStore(std::vector<UserAccount> accounts);

    /// The built-in default: single "admin"/"admin123" account with all
    /// permissions.  See docs/plan_przed_klientem.md Z2.3.
    static InMemoryUserStore with_default_admin();

    std::optional<UserAccount> authenticate(std::string_view login,
                                            std::string_view password) const override;

private:
    std::vector<UserAccount> accounts_;
};

}  // namespace server::terminal
