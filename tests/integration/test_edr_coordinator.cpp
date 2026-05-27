// tests/integration/test_edr_coordinator.cpp
//
// Integration tests for EdrCoordinator against a live PostgreSQL instance.
//
// Tests verify that EdrCoordinator correctly resolves the affected station
// for S25/S26 telegrams (SENT vs RECEIVED direction) and persists the result
// to session.edr_entries via PgDbWriter.
//
// Requires SYMULATOR_TEST_DB to be set (same as test_pg_db_writer.cpp).

#include "server/edr_coordinator.hpp"
#include "server/pg_db_writer.hpp"

#include <pqxx/pqxx>

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

using namespace engine::core;

namespace
{

static const char* db_conn_str()
{
    return std::getenv("SYMULATOR_TEST_DB");
}

// ── Fixture ───────────────────────────────────────────────────────────────────

class EdrCoordinatorFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* cs = db_conn_str();
        if (!cs)
            GTEST_SKIP() << "SYMULATOR_TEST_DB not set — skipping PostgreSQL integration tests";

        conn_str_ = cs;
        writer_ = std::make_unique<server::PgDbWriter>(conn_str_);
        session_uuid_ = writer_->init_session("test-edr-coordinator", 1);
        coordinator_ = std::make_unique<server::EdrCoordinator>(*writer_, session_uuid_);
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

    // Insert a PENDING edr_entries row so the UPDATE in EdrCoordinator has a target.
    void insert_pending_edr(const std::string& train, const std::string& station)
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        tx.exec(
            "INSERT INTO session.edr_entries "
            "  (session_id, train_number, station_sid, scheduled_departure, stop_type, status) "
            "VALUES ($1::uuid, $2, $3, make_interval(secs => 3600), 'COMMERCIAL', 'PENDING')",
            pqxx::params{session_uuid_, train, station});
        tx.commit();
    }

    std::string fetch_edr_status(const std::string& train, const std::string& station)
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        const auto r = tx.exec(
            "SELECT status FROM session.edr_entries "
            "WHERE session_id = $1::uuid AND train_number = $2 AND station_sid = $3",
            pqxx::params{session_uuid_, train, station});
        tx.commit();
        if (r.empty())
            return "";
        return r[0][0].as<std::string>();
    }

    std::string conn_str_;
    std::unique_ptr<server::PgDbWriter> writer_;
    std::unique_ptr<server::EdrCoordinator> coordinator_;
    std::string session_uuid_;
};

}  // namespace

// ── S25: departure ────────────────────────────────────────────────────────────

TEST_F(EdrCoordinatorFixture, S25_Sent_UsesSrcAreaAsStation)
{
    // SENT: operator at "GOR" sent S25 → departure recorded at GOR.
    insert_pending_edr("IC 101", "GOR");

    coordinator_->on_telegram_accepted(DispatchFormType::S25, TelegramDirection::SENT,
                                       /*src_area=*/"GOR", /*dst_area=*/"SOP",
                                       /*train_number=*/"IC 101",
                                       /*timestamp_us=*/3600ULL * 1'000'000ULL);

    EXPECT_EQ(fetch_edr_status("IC 101", "GOR"), "DEPARTED");
    // dst_area station row should be untouched.
    EXPECT_EQ(fetch_edr_status("IC 101", "SOP"), "");
}

TEST_F(EdrCoordinatorFixture, S25_Received_UsesDstAreaAsStation)
{
    // RECEIVED: operator at "SOP" recorded an incoming S25 → departure was at GOR (dst_area).
    insert_pending_edr("IC 202", "GOR");

    coordinator_->on_telegram_accepted(DispatchFormType::S25, TelegramDirection::RECEIVED,
                                       /*src_area=*/"SOP", /*dst_area=*/"GOR",
                                       /*train_number=*/"IC 202",
                                       /*timestamp_us=*/7200ULL * 1'000'000ULL);

    EXPECT_EQ(fetch_edr_status("IC 202", "GOR"), "DEPARTED");
}

// ── S26: arrival ──────────────────────────────────────────────────────────────

TEST_F(EdrCoordinatorFixture, S26_Sent_UsesSrcAreaAsStation)
{
    // SENT: operator at "SOP" sent S26 → arrival recorded at SOP.
    insert_pending_edr("IC 303", "SOP");

    coordinator_->on_telegram_accepted(DispatchFormType::S26, TelegramDirection::SENT,
                                       /*src_area=*/"SOP", /*dst_area=*/"GOR",
                                       /*train_number=*/"IC 303",
                                       /*timestamp_us=*/10800ULL * 1'000'000ULL);

    EXPECT_EQ(fetch_edr_status("IC 303", "SOP"), "ARRIVED");
}

TEST_F(EdrCoordinatorFixture, S26_Received_UsesDstAreaAsStation)
{
    // RECEIVED: operator at "GOR" recorded an incoming S26 → arrival was at SOP (dst_area).
    insert_pending_edr("IC 404", "SOP");

    coordinator_->on_telegram_accepted(DispatchFormType::S26, TelegramDirection::RECEIVED,
                                       /*src_area=*/"GOR", /*dst_area=*/"SOP",
                                       /*train_number=*/"IC 404",
                                       /*timestamp_us=*/14400ULL * 1'000'000ULL);

    EXPECT_EQ(fetch_edr_status("IC 404", "SOP"), "ARRIVED");
}

// ── Unhandled form types ──────────────────────────────────────────────────────

TEST_F(EdrCoordinatorFixture, OtherFormTypes_DoNotWriteToDb)
{
    // S2, S24, S55, S56 are not handled by EdrCoordinator.
    insert_pending_edr("IC 505", "GOR");

    coordinator_->on_telegram_accepted(DispatchFormType::S2, TelegramDirection::SENT, "GOR", "SOP",
                                       "IC 505", 3600ULL * 1'000'000ULL);

    // Status must remain PENDING — coordinator must not update it.
    EXPECT_EQ(fetch_edr_status("IC 505", "GOR"), "PENDING");
}
