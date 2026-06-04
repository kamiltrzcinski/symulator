#pragma once

#include <gtest/gtest.h>

#include <pqxx/pqxx>

#include <cstdlib>
#include <string>
#include <utility>

namespace tests::common
{

class DbIntegrationFixture : public ::testing::Test
{
public:
    explicit DbIntegrationFixture(std::string session_display_name)
        : session_display_name_(std::move(session_display_name))
    {
    }

protected:
    void SetUp() override
    {
        const char* cs = std::getenv("SYMULATOR_TEST_DB");
        if (!cs)
        {
            GTEST_SKIP() << "SYMULATOR_TEST_DB not set — skipping PostgreSQL integration tests";
        }

        conn_str_ = cs;
    }

    void TearDown() override
    {
        if (conn_str_.empty() || session_uuid_.empty())
            return;

        try
        {
            pqxx::connection c{conn_str_};
            pqxx::work tx{c};
            tx.exec("DELETE FROM session.sessions WHERE id = $1::uuid",
                    pqxx::params{session_uuid_});
            tx.commit();
        }
        catch (...)
        {
        }
    }

    const std::string& session_display_name() const { return session_display_name_; }

    std::string conn_str_;
    std::string session_uuid_;

private:
    std::string session_display_name_;
};

}  // namespace tests::common
