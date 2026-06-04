// tests/integration/test_pip_track_state.cpp
//
// Integration tests for PgDbWriter::upsert_pip_track_state() against a live
// PostgreSQL instance.  Verifies INSERT and ON CONFLICT DO UPDATE semantics
// on the pip.track_state table.
//
// Requires SYMULATOR_TEST_DB to be set.

#include "server/pg_db_writer.hpp"

#include <tests/common/db_integration_fixture.hpp>

#include <pqxx/pqxx>

#include <gtest/gtest.h>

#include <string>

namespace
{

static constexpr uint64_t kSecA = 200001ULL;
static constexpr uint64_t kSecB = 200002ULL;
static constexpr uint64_t kSecC = 200003ULL;
static constexpr uint64_t kSecD = 200004ULL;
static constexpr uint64_t kSecE = 200005ULL;

// ── Fixture ───────────────────────────────────────────────────────────────────

class PipTrackStateFixture : public tests::common::DbIntegrationFixture
{
public:
    PipTrackStateFixture() : DbIntegrationFixture("test-pip-track-state") {}

protected:
    void SetUp() override
    {
        DbIntegrationFixture::SetUp();
        if (conn_str_.empty())
            return;

        writer_ = std::make_unique<server::PgDbWriter>(conn_str_);
        session_uuid_ = writer_->init_session(session_display_name(), 1);
    }

    // Fetch the raw trains JSON text for a given section.
    // Returns an empty string when no row exists.
    std::string fetch_trains_json(uint64_t section_uid)
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        const auto r = tx.exec(
            "SELECT trains::text FROM pip.track_state "
            "WHERE session_id = $1::uuid AND section_uid = $2::bigint",
            pqxx::params{session_uuid_, static_cast<int64_t>(section_uid)});
        tx.commit();
        if (r.empty())
            return "";
        return r[0][0].as<std::string>();
    }

    std::unique_ptr<server::PgDbWriter> writer_;
};

}  // namespace

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST_F(PipTrackStateFixture, Upsert_InsertsNewRow)
{
    const std::string trains =
        R"([{"number":"IC 101","has_extra_info":false,"manually_placed":false,"entry_side":"LEFT"}])";

    writer_->upsert_pip_track_state(session_uuid_, kSecA, trains);

    const std::string stored = fetch_trains_json(kSecA);
    EXPECT_FALSE(stored.empty()) << "Row should have been inserted";
}

TEST_F(PipTrackStateFixture, Upsert_FreeSection_StoresEmptyArray)
{
    writer_->upsert_pip_track_state(session_uuid_, kSecB, "[]");

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT jsonb_array_length(trains) FROM pip.track_state "
        "WHERE session_id = $1::uuid AND section_uid = $2::bigint",
        pqxx::params{session_uuid_, static_cast<int64_t>(kSecB)});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<int>(), 0);
}

TEST_F(PipTrackStateFixture, Upsert_ConflictUpdatesTrains)
{
    const std::string trains_v1 =
        R"([{"number":"IC 101","has_extra_info":false,"manually_placed":false,"entry_side":"LEFT"}])";
    const std::string trains_v2 = "[]";

    // First upsert: occupied
    writer_->upsert_pip_track_state(session_uuid_, kSecC, trains_v1);

    // Second upsert: now free — ON CONFLICT DO UPDATE must overwrite.
    writer_->upsert_pip_track_state(session_uuid_, kSecC, trains_v2);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT jsonb_array_length(trains) FROM pip.track_state "
        "WHERE session_id = $1::uuid AND section_uid = $2::bigint",
        pqxx::params{session_uuid_, static_cast<int64_t>(kSecC)});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<int>(), 0) << "trains should have been updated to empty array";
}

TEST_F(PipTrackStateFixture, Upsert_MultipleSections_AllInserted)
{
    writer_->upsert_pip_track_state(session_uuid_, kSecA, "[]");
    writer_->upsert_pip_track_state(session_uuid_, kSecB, "[]");
    writer_->upsert_pip_track_state(session_uuid_, kSecC, "[]");

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec("SELECT COUNT(*) FROM pip.track_state WHERE session_id = $1::uuid",
                           pqxx::params{session_uuid_});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<int64_t>(), 3LL);
}

TEST_F(PipTrackStateFixture, Upsert_UpdatedAtChangesOnConflict)
{
    writer_->upsert_pip_track_state(session_uuid_, kSecD, "[]");

    // Fetch original updated_at
    std::string first_ts;
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        const auto r = tx.exec(
            "SELECT updated_at FROM pip.track_state "
            "WHERE session_id = $1::uuid AND section_uid = $2::bigint",
            pqxx::params{session_uuid_, static_cast<int64_t>(kSecD)});
        tx.commit();
        ASSERT_EQ(r.size(), 1u);
        first_ts = r[0][0].as<std::string>();
    }

    writer_->upsert_pip_track_state(session_uuid_, kSecD, "[]");

    // updated_at must not be null after the second upsert.
    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT updated_at IS NOT NULL FROM pip.track_state "
        "WHERE session_id = $1::uuid AND section_uid = $2::bigint",
        pqxx::params{session_uuid_, static_cast<int64_t>(kSecD)});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_TRUE(r[0][0].as<bool>());
}
