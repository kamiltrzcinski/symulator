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
            tx.exec("DELETE FROM session.sessions WHERE id = $1::uuid",
                    pqxx::params{session_uuid_});
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
    const auto r = tx.exec("SELECT display_name, status FROM session.sessions WHERE id = $1::uuid",
                           pqxx::params{session_uuid_});
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
    const auto r = tx.exec(
        "SELECT form_type, exchange_id, train_number, from_sid, to_sid, direction, status "
        "FROM session.dispatch_telegrams "
        "WHERE session_id = $1::uuid",
        pqxx::params{session_uuid_});
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
    const auto r = tx.exec(
        "SELECT track_number IS NULL FROM session.dispatch_telegrams "
        "WHERE session_id = $1::uuid",
        pqxx::params{session_uuid_});
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
        tx.exec(
            "INSERT INTO session.edr_entries "
            "  (session_id, train_number, station_sid, scheduled_departure, stop_type, status) "
            "VALUES ($1::uuid, $2, $3, '01:00:00'::interval, 'COMMERCIAL', 'PENDING')",
            pqxx::params{session_uuid_, "IC 12345", "SOP"});
        tx.commit();
    }

    // 13:45:00 expressed as microseconds since Unix epoch (on 2024-05-24 = 1716559500 s)
    constexpr std::uint64_t ts_us = 1'716'559'500'000'000ULL;

    writer_->update_edr_track_clear_time(session_uuid_, "IC 12345", "SOP", ts_us);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT track_clear_time IS NOT NULL FROM session.edr_entries "
        "WHERE session_id = $1::uuid AND train_number = $2 AND station_sid = $3",
        pqxx::params{session_uuid_, "IC 12345", "SOP"});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_TRUE(r[0][0].as<bool>()) << "track_clear_time should have been set";
}

TEST_F(PgDbWriterFixture, WriteDomainEvent_InsertsRow)
{
    // Simulate a serialized FlatBuffers body (minimal non-empty bytes).
    const std::vector<uint8_t> fake_payload{0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};

    server::DomainEventRow row;
    row.event_type = 0x01;  // SwitchPositionChanged
    row.event_id = 42;
    row.timestamp_us = 1'716'559'500'000'000ULL;
    row.object_gid = "ZWR-TRJ-GOr-zwr1";
    row.payload = fake_payload;

    writer_->write_domain_event(session_uuid_, row);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT event_type, event_id, object_gid, octet_length(payload) "
        "FROM session.events "
        "WHERE session_id = $1::uuid",
        pqxx::params{session_uuid_});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<int>(), 0x01);
    EXPECT_EQ(r[0][1].as<int64_t>(), 42);
    EXPECT_EQ(r[0][2].as<std::string>(), "ZWR-TRJ-GOr-zwr1");
    EXPECT_EQ(r[0][3].as<int>(), static_cast<int>(fake_payload.size()));
}

TEST_F(PgDbWriterFixture, WriteDomainEvent_NullObjectGid)
{
    server::DomainEventRow row;
    row.event_type = 0x07;  // RouteSet (session-level, no single object GID)
    row.event_id = 99;
    row.timestamp_us = 1'716'559'501'000'000ULL;
    // object_gid left as std::nullopt
    row.payload = {0x04, 0x00, 0x00, 0x00};

    writer_->write_domain_event(session_uuid_, row);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT object_gid IS NULL FROM session.events "
        "WHERE session_id = $1::uuid AND event_id = $2",
        pqxx::params{session_uuid_, static_cast<int64_t>(99)});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_TRUE(r[0][0].as<bool>()) << "object_gid should be NULL when not set";
}

// ── EDR departure / arrival ───────────────────────────────────────────────────

TEST_F(PgDbWriterFixture, UpdateEdrDeparture_SetsActualDepartureAndStatus)
{
    // Insert a PENDING EDR entry directly so the UPDATE has a target row.
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        tx.exec(
            "INSERT INTO session.edr_entries "
            "  (session_id, train_number, station_sid, scheduled_departure, status) "
            "VALUES ($1::uuid, $2, $3, make_interval(secs => 3600), 'PENDING')",
            pqxx::params{session_uuid_, "IC 1001", "ZWR"});
        tx.commit();
    }

    // ~01:00:30 — 3630 seconds into the day.
    constexpr uint64_t ts = 3630ULL * 1'000'000ULL;
    writer_->update_edr_departure(session_uuid_, "IC 1001", "ZWR", ts);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT status, "
        "       EXTRACT(EPOCH FROM actual_departure)::bigint AS secs "
        "FROM session.edr_entries "
        "WHERE session_id = $1::uuid AND train_number = $2 AND station_sid = $3",
        pqxx::params{session_uuid_, "IC 1001", "ZWR"});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0]["status"].as<std::string>(), "DEPARTED");
    EXPECT_EQ(r[0]["secs"].as<int64_t>(), 3630LL);
}

TEST_F(PgDbWriterFixture, UpdateEdrArrival_SetsActualArrivalAndStatus)
{
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        tx.exec(
            "INSERT INTO session.edr_entries "
            "  (session_id, train_number, station_sid, scheduled_departure, status) "
            "VALUES ($1::uuid, $2, $3, make_interval(secs => 7200), 'PENDING')",
            pqxx::params{session_uuid_, "TLK 2002", "SOP"});
        tx.commit();
    }

    // ~02:00:15 — 7215 seconds into the day.
    constexpr uint64_t ts = 7215ULL * 1'000'000ULL;
    writer_->update_edr_arrival(session_uuid_, "TLK 2002", "SOP", ts);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT status, "
        "       EXTRACT(EPOCH FROM actual_arrival)::bigint AS secs "
        "FROM session.edr_entries "
        "WHERE session_id = $1::uuid AND train_number = $2 AND station_sid = $3",
        pqxx::params{session_uuid_, "TLK 2002", "SOP"});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0]["status"].as<std::string>(), "ARRIVED");
    EXPECT_EQ(r[0]["secs"].as<int64_t>(), 7215LL);
}

TEST_F(PgDbWriterFixture, UpdateEdrDeparture_IdempotentOnAlreadyDeparted)
{
    // Pre-insert row already in DEPARTED state.
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        tx.exec(
            "INSERT INTO session.edr_entries "
            "  (session_id, train_number, station_sid, scheduled_departure, "
            "   actual_departure, status) "
            "VALUES ($1::uuid, $2, $3, make_interval(secs => 1000), "
            "        make_interval(secs => 1010), 'DEPARTED')",
            pqxx::params{session_uuid_, "EX 3003", "GDY"});
        tx.commit();
    }

    constexpr uint64_t ts2 = 9999ULL * 1'000'000ULL;
    writer_->update_edr_departure(session_uuid_, "EX 3003", "GDY", ts2);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT EXTRACT(EPOCH FROM actual_departure)::bigint AS secs "
        "FROM session.edr_entries "
        "WHERE session_id = $1::uuid AND train_number = $2 AND station_sid = $3",
        pqxx::params{session_uuid_, "EX 3003", "GDY"});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    // Second call must NOT overwrite the original 1010-second value.
    EXPECT_EQ(r[0]["secs"].as<int64_t>(), 1010LL);
}

// ── save_snapshot ─────────────────────────────────────────────────────────────

TEST_F(PgDbWriterFixture, SaveSnapshot_InsertsRow)
{
    const std::vector<std::uint8_t> payload{0x01, 0x02, 0x03, 0x04};
    writer_->save_snapshot(session_uuid_, 42LL, 1'000'000LL, payload);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT seq_cursor, timestamp_us, octet_length(payload) "
        "FROM session.snapshots "
        "WHERE session_id = $1::uuid",
        pqxx::params{session_uuid_});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<int64_t>(), 42LL);
    EXPECT_EQ(r[0][1].as<int64_t>(), 1'000'000LL);
    EXPECT_EQ(r[0][2].as<int>(), static_cast<int>(payload.size()));
}

// ── append_chat_message ───────────────────────────────────────────────────────

TEST_F(PgDbWriterFixture, AppendChatMessage_InsertsRow)
{
    writer_->append_chat_message(session_uuid_, "player-1", "BROADCAST", std::nullopt,
                                 "Hello world", 5'000LL);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT sender_id, target_type, target_id IS NULL, body, timestamp_us "
        "FROM session.chat_log "
        "WHERE session_id = $1::uuid",
        pqxx::params{session_uuid_});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<std::string>(), "player-1");
    EXPECT_EQ(r[0][1].as<std::string>(), "BROADCAST");
    EXPECT_TRUE(r[0][2].as<bool>()) << "target_id should be NULL for BROADCAST";
    EXPECT_EQ(r[0][3].as<std::string>(), "Hello world");
    EXPECT_EQ(r[0][4].as<int64_t>(), 5'000LL);
}

// ── assign_operating_point / release_operating_point ────────────────────────

TEST_F(PgDbWriterFixture, AssignOperatingPoint_InsertsRow)
{
    const int64_t id =
        writer_->assign_operating_point(session_uuid_, "OP-GDN-1", "GDN", "player-1");

    EXPECT_GT(id, 0);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT operating_point_id, station_sid, client_id, released_at IS NULL "
        "FROM session.operating_point_assignments "
        "WHERE session_id = $1::uuid",
        pqxx::params{session_uuid_});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<std::string>(), "OP-GDN-1");
    EXPECT_EQ(r[0][1].as<std::string>(), "GDN");
    EXPECT_EQ(r[0][2].as<std::string>(), "player-1");
    EXPECT_TRUE(r[0][3].as<bool>()) << "released_at should be NULL when still held";
}

TEST_F(PgDbWriterFixture, ReleaseOperatingPoint_SetsReleasedAt)
{
    writer_->assign_operating_point(session_uuid_, "OP-SOP-2", "SOP", "player-2");
    writer_->release_operating_point(session_uuid_, "OP-SOP-2", "player-2");

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT released_at IS NOT NULL "
        "FROM session.operating_point_assignments "
        "WHERE session_id = $1::uuid AND operating_point_id = $2",
        pqxx::params{session_uuid_, "OP-SOP-2"});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_TRUE(r[0][0].as<bool>()) << "released_at should be set after release";
}

TEST_F(PgDbWriterFixture, ReleaseOperatingPoint_Idempotent)
{
    writer_->assign_operating_point(session_uuid_, "OP-GOR-3", "GOR", "player-3");
    writer_->release_operating_point(session_uuid_, "OP-GOR-3", "player-3");

    // Second release must not throw and must leave exactly one row.
    EXPECT_NO_THROW(writer_->release_operating_point(session_uuid_, "OP-GOR-3", "player-3"));

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT COUNT(*) FROM session.operating_point_assignments "
        "WHERE session_id = $1::uuid AND operating_point_id = $2",
        pqxx::params{session_uuid_, "OP-GOR-3"});
    tx.commit();

    EXPECT_EQ(r[0][0].as<int64_t>(), 1LL);
}

// ── upsert_timetable_template ─────────────────────────────────────────────────

TEST_F(PgDbWriterFixture, UpsertTimetableTemplate_InsertsRow)
{
    writer_->upsert_timetable_template("IC-1234", "SOP", std::nullopt, "3600", "3660",
                                       std::string{"T1"}, "COMMERCIAL");

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT train_number, station_sid, stop_type, "
        "       EXTRACT(EPOCH FROM scheduled_departure)::bigint AS scheduled_departure_secs, "
        "       track_number "
        "FROM fleet.timetable_templates "
        "WHERE train_number = $1 AND station_sid = $2",
        pqxx::params{"IC-1234", "SOP"});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<std::string>(), "IC-1234");
    EXPECT_EQ(r[0][1].as<std::string>(), "SOP");
    EXPECT_EQ(r[0][2].as<std::string>(), "COMMERCIAL");
    EXPECT_EQ(r[0][3].as<int64_t>(), 3660LL);
    EXPECT_EQ(r[0][4].as<std::string>(), "T1");
}

}  // namespace
