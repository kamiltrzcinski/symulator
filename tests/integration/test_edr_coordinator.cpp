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

#include <tests/common/db_integration_fixture.hpp>

#include <pqxx/pqxx>

#include <gtest/gtest.h>

#include <string>

using namespace engine::core;

namespace
{

static constexpr uint64_t kGorUid = 3001ULL;
static constexpr uint64_t kSopUid = 3002ULL;

// ── Fixture ───────────────────────────────────────────────────────────────────

class EdrCoordinatorFixture : public tests::common::DbIntegrationFixture
{
public:
    EdrCoordinatorFixture() : DbIntegrationFixture("test-edr-coordinator") {}

protected:
    void SetUp() override
    {
        DbIntegrationFixture::SetUp();
        if (conn_str_.empty())
            return;

        writer_ = std::make_unique<server::PgDbWriter>(conn_str_);
        session_uuid_ = writer_->init_session(session_display_name(), 1);
        coordinator_ = std::make_unique<server::EdrCoordinator>(*writer_, session_uuid_);
    }

    // Insert a PENDING edr_entries row so the UPDATE in EdrCoordinator has a target.
    void insert_pending_edr(const std::string& train, uint64_t station_uid)
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        tx.exec(
            "INSERT INTO session.edr_entries "
            "  (session_id, train_number, station_uid, scheduled_departure, stop_type, status) "
            "VALUES ($1::uuid, $2, $3::bigint, make_interval(secs => 3600), 'COMMERCIAL', "
            "'PENDING')",
            pqxx::params{session_uuid_, train, static_cast<int64_t>(station_uid)});
        tx.commit();
    }

    std::string fetch_edr_status(const std::string& train, uint64_t station_uid)
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        const auto r = tx.exec(
            "SELECT status FROM session.edr_entries "
            "WHERE session_id = $1::uuid AND train_number = $2 AND station_uid = $3::bigint",
            pqxx::params{session_uuid_, train, static_cast<int64_t>(station_uid)});
        tx.commit();
        if (r.empty())
            return "";
        return r[0][0].as<std::string>();
    }

    std::unique_ptr<server::PgDbWriter> writer_;
    std::unique_ptr<server::EdrCoordinator> coordinator_;
};

}  // namespace

// ── S25: departure ────────────────────────────────────────────────────────────

TEST_F(EdrCoordinatorFixture, S25_Sent_UsesSrcAreaAsStation)
{
    insert_pending_edr("IC 101", kGorUid);

    coordinator_->on_telegram_accepted(DispatchFormType::S25, TelegramDirection::SENT,
                                       /*src_area=*/std::to_string(kGorUid),
                                       /*dst_area=*/std::to_string(kSopUid),
                                       /*train_number=*/"IC 101",
                                       /*timestamp_us=*/3600ULL * 1'000'000ULL);

    EXPECT_EQ(fetch_edr_status("IC 101", kGorUid), "DEPARTED");
    EXPECT_EQ(fetch_edr_status("IC 101", kSopUid), "");
}

TEST_F(EdrCoordinatorFixture, S25_Received_UsesDstAreaAsStation)
{
    // RECEIVED: operator at kSopUid recorded an incoming S25 → departure was at kGorUid (dst_area).
    insert_pending_edr("IC 202", kGorUid);

    coordinator_->on_telegram_accepted(DispatchFormType::S25, TelegramDirection::RECEIVED,
                                       /*src_area=*/std::to_string(kSopUid),
                                       /*dst_area=*/std::to_string(kGorUid),
                                       /*train_number=*/"IC 202",
                                       /*timestamp_us=*/7200ULL * 1'000'000ULL);

    EXPECT_EQ(fetch_edr_status("IC 202", kGorUid), "DEPARTED");
}

// ── S26: arrival ──────────────────────────────────────────────────────────────

TEST_F(EdrCoordinatorFixture, S26_Sent_UsesSrcAreaAsStation)
{
    insert_pending_edr("IC 303", kSopUid);

    coordinator_->on_telegram_accepted(DispatchFormType::S26, TelegramDirection::SENT,
                                       /*src_area=*/std::to_string(kSopUid),
                                       /*dst_area=*/std::to_string(kGorUid),
                                       /*train_number=*/"IC 303",
                                       /*timestamp_us=*/10800ULL * 1'000'000ULL);

    EXPECT_EQ(fetch_edr_status("IC 303", kSopUid), "ARRIVED");
}

TEST_F(EdrCoordinatorFixture, S26_Received_UsesDstAreaAsStation)
{
    // RECEIVED: operator at kGorUid recorded an incoming S26 → arrival was at kSopUid (dst_area).
    insert_pending_edr("IC 404", kSopUid);

    coordinator_->on_telegram_accepted(DispatchFormType::S26, TelegramDirection::RECEIVED,
                                       /*src_area=*/std::to_string(kGorUid),
                                       /*dst_area=*/std::to_string(kSopUid),
                                       /*train_number=*/"IC 404",
                                       /*timestamp_us=*/14400ULL * 1'000'000ULL);

    EXPECT_EQ(fetch_edr_status("IC 404", kSopUid), "ARRIVED");
}

// ── Unhandled form types ──────────────────────────────────────────────────────

TEST_F(EdrCoordinatorFixture, OtherFormTypes_DoNotWriteToDb)
{
    // S2, S24, S55, S56 are not handled by EdrCoordinator.
    insert_pending_edr("IC 505", kGorUid);

    coordinator_->on_telegram_accepted(DispatchFormType::S2, TelegramDirection::SENT,
                                       std::to_string(kGorUid), std::to_string(kSopUid), "IC 505",
                                       3600ULL * 1'000'000ULL);

    // Status must remain PENDING — coordinator must not update it.
    EXPECT_EQ(fetch_edr_status("IC 505", kGorUid), "PENDING");
}
