// server/src/terminal/user_store.cpp

#include "server/terminal/user_store.hpp"

#include <algorithm>
#include <utility>

namespace server::terminal
{

InMemoryUserStore::InMemoryUserStore(std::vector<UserAccount> accounts)
    : accounts_(std::move(accounts))
{
}

InMemoryUserStore InMemoryUserStore::with_default_admin()
{
    return InMemoryUserStore({UserAccount{
        .login = "admin",
        .password = "admin123",
        .permissions = all_permissions(),
    }});
}

std::optional<UserAccount> InMemoryUserStore::authenticate(std::string_view login,
                                                           std::string_view password) const
{
    auto it = std::find_if(accounts_.begin(), accounts_.end(), [&](const UserAccount& a)
                           { return a.login == login && a.password == password; });
    if (it == accounts_.end())
        return std::nullopt;
    return *it;
}

}  // namespace server::terminal
