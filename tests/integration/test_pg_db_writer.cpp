// tests/integration/test_pg_db_writer.cpp
//
// Integration tests for PgDbWriter.
// These tests require a live PostgreSQL instance.
//
// Set SYMULATOR_TEST_DB to a libpq connection string before running:
//   export SYMULATOR_TEST_DB="host=localhost port=5432 dbname=symulator user=symulator password=secret"
//   ctest -R PgDbWriter
//
// All tests are automatically SKIPPED when SYMULATOR_TEST_DB is not set.
// Clean-up: each test deletes its own session row (CASCADE removes related rows).

#include "server/pg_db_writer.hpp"

#include <pqxx/pqxx>

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

namespace
{

static const char* db_conn_str()
{
    return std::getenv("SYMULATOR_TEST_DB");
}

// ── Fixture ───────────────────────────────────────────────────────────────────

class PgDbWriterFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* cs = db_conn_str();
        if (!cs)
            GTEST_SKIP() << "SYMULATOR_TEST_DB not set — skipping PostgreSQL integration tests";

        conn_str_ = cs;
        writer_ = std::make_unique<server::PgDbWriter>(conn_str_);
        session_uuid_ = writer_->init_session("test-integration", 1);
    }

    void TearDown() override
    {
        if (conn_str_.empty() || session_uuid_.empty())
            return;
        try
        {
            pqxx::connection c{conn_str_};
            pqxx::work tx{c};
            // CASCADE handles dispatch_telegrams and edr_entries rows.
            tx.exec_params("DELETE FROM session.sessions WHERE id = $1::uuid", session_uuid_);
            tx.commit();
        }
        catch (...)
        {
        }
    }

    std::string conn_str_;
    std::unique_ptr<server::PgDbWriter> writer_;
    std::string session_uuid_;
};

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST_F(PgDbWriterFixture, InitSession_CreatesSessionsRow)
{
    ASSERT_FALSE(session_uuid_.empty());

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec_params(
        "SELECT display_name, status FROM session.sessions WHERE id = $1::uuid", session_uuid_);
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<std::string>(), "test-integration");
    EXPECT_EQ(r[0][1].as<std::string>(), "STARTED");
}

TEST_F(PgDbWriterFixture, WriteDispatchTelegram_InsertsRow)
{
    server::TelegramRow row;
    row.form_type = "S2";
    row.exchange_id = "exch-0000001";
    row.train_number = "IC 12345";
    row.from_sid = "GOR";
    row.to_sid = "SOP";
    row.direction = "SENT";
    row.status = "ACCEPTED";
    row.km_markers = {"210.394", "212.705"};
    row.body = "{}";
    row.timestamp_us = 1'716'500'000'000'000ULL;

    writer_->write_dispatch_telegram(session_uuid_, row);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec_params(
        "SELECT form_type, exchange_id, train_number, from_sid, to_sid, direction, status "
        "FROM session.dispatch_telegrams "
        "WHERE session_id = $1::uuid",
        session_uuid_);
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<std::string>(), "S2");
    EXPECT_EQ(r[0][1].as<std::string>(), "exch-0000001");
    EXPECT_EQ(r[0][2].as<std::string>(), "IC 12345");
    EXPECT_EQ(r[0][3].as<std::string>(), "GOR");
    EXPECT_EQ(r[0][4].as<std::string>(), "SOP");
    EXPECT_EQ(r[0][5].as<std::string>(), "SENT");
    EXPECT_EQ(r[0][6].as<std::string>(), "ACCEPTED");
}

TEST_F(PgDbWriterFixture, WriteDispatchTelegram_NullableTrackNumber)
{
    server::TelegramRow row;
    row.form_type = "FREE_TEXT";
    row.exchange_id = "";
    row.train_number = "";
    row.from_sid = "GOR";
    row.to_sid = "SOP";
    row.direction = "SENT";
    row.status = "ACCEPTED";
    // track_number not set → std::nullopt → NULL in DB
    row.body = "hello from GOR";
    row.timestamp_us = 1'716'500'001'000'000ULL;

    writer_->write_dispatch_telegram(session_uuid_, row);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec_params(
        "SELECT track_number IS NULL FROM session.dispatch_telegrams "
        "WHERE session_id = $1::uuid",
        session_uuid_);
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_TRUE(r[0][0].as<bool>());
}

TEST_F(PgDbWriterFixture, UpdateEdrTrackClearTime_UpdatesRow)
{
    // Insert an edr_entries row so the UPDATE has something to touch.
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        tx.exec_params(
            "INSERT INTO session.edr_entries "
            "  (session_id, train_number, station_sid, scheduled_departure, stop_type, status) "
            "VALUES ($1::uuid, $2, $3, '01:00:00'::interval, 'COMMERCIAL', 'PENDING')",
            session_uuid_, "IC 12345", "SOP");
        tx.commit();
    }

    // 13:45:00 expressed as microseconds since Unix epoch (on 2024-05-24 = 1716559500 s)
    constexpr std::uint64_t ts_us = 1'716'559'500'000'000ULL;

    writer_->update_edr_track_clear_time(session_uuid_, "IC 12345", "SOP", ts_us);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec_params(
        "SELECT track_clear_time IS NOT NULL FROM session.edr_entries "
        "WHERE session_id = $1::uuid AND train_number = $2 AND station_sid = $3",
        session_uuid_, "IC 12345", "SOP");
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_TRUE(r[0][0].as<bool>()) << "track_clear_time should have been set";
}

}  // namespace
