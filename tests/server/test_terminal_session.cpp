// tests/server/test_terminal_session.cpp
//
// Tests for the built-in terminal: login/logout, permission enforcement, and
// help filtering.  Uses a fake command so no engine wiring is needed.

#include "server/terminal/command_registry.hpp"
#include "server/terminal/terminal_session.hpp"
#include "server/terminal/user_store.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>

namespace
{

using namespace server::terminal;

// Records executions; requires a configurable permission.
class FakeCommand final : public ITerminalCommand
{
public:
    FakeCommand(std::string name, Permission perm, int& counter)
        : name_(std::move(name)), perm_(perm), counter_(counter)
    {
    }

    std::string_view name() const override { return name_; }
    std::string_view help() const override { return name_; }
    Permission required_permission() const override { return perm_; }

    std::string execute(const std::vector<std::string>& args) override
    {
        ++counter_;
        return name_ + " executed with " + std::to_string(args.size()) + " args";
    }

private:
    std::string name_;
    Permission perm_;
    int& counter_;
};

struct SessionFixture
{
    int spawn_calls = 0;
    int reset_calls = 0;
    CommandRegistry registry;
    InMemoryUserStore users{{
        UserAccount{"admin", "admin123", all_permissions()},
        UserAccount{"moderator",
                    "mod123",
                    {Permission::SPAWN, Permission::DESPAWN, Permission::KICK, Permission::BAN,
                     Permission::LOGS}},
    }};

    SessionFixture()
    {
        registry.add(std::make_unique<FakeCommand>("spawn", Permission::SPAWN, spawn_calls));
        registry.add(std::make_unique<FakeCommand>("reset", Permission::RESET, reset_calls));
    }
};

TEST(TerminalSession, CommandsRequireLogin)
{
    SessionFixture f;
    TerminalSession session(f.registry, f.users);

    const auto out = session.process_line("spawn x y");
    EXPECT_NE(out.find("not logged in"), std::string::npos);
    EXPECT_EQ(f.spawn_calls, 0);
}

TEST(TerminalSession, LoginWithDefaultAdminCredentials)
{
    SessionFixture f;
    TerminalSession session(f.registry, f.users);

    EXPECT_FALSE(session.is_authenticated());
    const auto out = session.process_line("login admin admin123");
    EXPECT_NE(out.find("logged in as admin"), std::string::npos);
    EXPECT_TRUE(session.is_authenticated());
}

TEST(TerminalSession, LoginRejectsBadCredentials)
{
    SessionFixture f;
    TerminalSession session(f.registry, f.users);

    EXPECT_EQ(session.process_line("login admin wrong"), "invalid credentials");
    EXPECT_FALSE(session.is_authenticated());
}

TEST(TerminalSession, AdminCanRunEverything)
{
    SessionFixture f;
    TerminalSession session(f.registry, f.users);
    session.process_line("login admin admin123");

    session.process_line("spawn a b");
    session.process_line("reset");
    EXPECT_EQ(f.spawn_calls, 1);
    EXPECT_EQ(f.reset_calls, 1);
}

TEST(TerminalSession, ModeratorDeniedResetButAllowedSpawn)
{
    SessionFixture f;
    TerminalSession session(f.registry, f.users);
    session.process_line("login moderator mod123");

    session.process_line("spawn a b");
    EXPECT_EQ(f.spawn_calls, 1);

    const auto out = session.process_line("reset");
    EXPECT_NE(out.find("permission denied"), std::string::npos);
    EXPECT_NE(out.find("RESET"), std::string::npos);
    EXPECT_EQ(f.reset_calls, 0);
}

TEST(TerminalSession, LogoutDropsPermissions)
{
    SessionFixture f;
    TerminalSession session(f.registry, f.users);
    session.process_line("login admin admin123");
    session.process_line("logout");

    EXPECT_FALSE(session.is_authenticated());
    const auto out = session.process_line("spawn a b");
    EXPECT_NE(out.find("not logged in"), std::string::npos);
}

TEST(TerminalSession, UnknownCommandReported)
{
    SessionFixture f;
    TerminalSession session(f.registry, f.users);
    session.process_line("login admin admin123");

    const auto out = session.process_line("frobnicate");
    EXPECT_NE(out.find("unknown command"), std::string::npos);
}

TEST(TerminalSession, HelpFiltersByPermission)
{
    SessionFixture f;
    TerminalSession session(f.registry, f.users);

    // Before login: only built-ins.
    auto out = session.process_line("help");
    EXPECT_NE(out.find("login"), std::string::npos);
    EXPECT_EQ(out.find("reset"), std::string::npos);

    // Moderator: sees spawn, not reset.
    session.process_line("login moderator mod123");
    out = session.process_line("help");
    EXPECT_NE(out.find("spawn"), std::string::npos);
    EXPECT_EQ(out.find("reset"), std::string::npos);

    // Admin: sees everything.
    session.process_line("logout");
    session.process_line("login admin admin123");
    out = session.process_line("help");
    EXPECT_NE(out.find("spawn"), std::string::npos);
    EXPECT_NE(out.find("reset"), std::string::npos);
}

TEST(TerminalSession, EmptyLineProducesNoOutput)
{
    SessionFixture f;
    TerminalSession session(f.registry, f.users);
    EXPECT_EQ(session.process_line(""), "");
    EXPECT_EQ(session.process_line("   "), "");
}

TEST(CommandRegistry, DuplicateNameThrows)
{
    int counter = 0;
    CommandRegistry registry;
    registry.add(std::make_unique<FakeCommand>("spawn", Permission::SPAWN, counter));
    EXPECT_THROW(registry.add(std::make_unique<FakeCommand>("spawn", Permission::SPAWN, counter)),
                 std::invalid_argument);
}

TEST(InMemoryUserStore, DefaultAdminFactory)
{
    const auto store = InMemoryUserStore::with_default_admin();
    const auto account = store.authenticate("admin", "admin123");
    ASSERT_TRUE(account.has_value());
    EXPECT_EQ(account->permissions, all_permissions());
    EXPECT_FALSE(store.authenticate("admin", "nope").has_value());
}

}  // namespace
