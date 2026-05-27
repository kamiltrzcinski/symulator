// tests/integration/test_pip_track_state.cpp
//
// Integration tests for PgDbWriter::upsert_pip_track_state() against a live
// PostgreSQL instance.  Verifies INSERT and ON CONFLICT DO UPDATE semantics
// on the pip.track_state table.
//
// Requires SYMULATOR_TEST_DB to be set.

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

class PipTrackStateFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        const char* cs = db_conn_str();
        if (!cs)
            GTEST_SKIP() << "SYMULATOR_TEST_DB not set — skipping PostgreSQL integration tests";

        conn_str_ = cs;
        writer_ = std::make_unique<server::PgDbWriter>(conn_str_);
        session_uuid_ = writer_->init_session("test-pip-track-state", 1);
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

    // Fetch the raw trains JSON text for a given section.
    // Returns an empty string when no row exists.
    std::string fetch_trains_json(const std::string& section_gid)
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        const auto r = tx.exec(
            "SELECT trains::text FROM pip.track_state "
            "WHERE session_id = $1::uuid AND section_gid = $2",
            pqxx::params{session_uuid_, section_gid});
        tx.commit();
        if (r.empty())
            return "";
        return r[0][0].as<std::string>();
    }

    std::string conn_str_;
    std::unique_ptr<server::PgDbWriter> writer_;
    std::string session_uuid_;
};

}  // namespace

// ── Tests ─────────────────────────────────────────────────────────────────────

TEST_F(PipTrackStateFixture, Upsert_InsertsNewRow)
{
    const std::string section = "OT-tor_a";
    const std::string trains =
        R"([{"number":"IC 101","has_extra_info":false,"manually_placed":false,"entry_side":"LEFT"}])";

    writer_->upsert_pip_track_state(session_uuid_, section, trains);

    const std::string stored = fetch_trains_json(section);
    EXPECT_FALSE(stored.empty()) << "Row should have been inserted";
}

TEST_F(PipTrackStateFixture, Upsert_FreeSection_StoresEmptyArray)
{
    const std::string section = "OT-tor_b";

    writer_->upsert_pip_track_state(session_uuid_, section, "[]");

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT jsonb_array_length(trains) FROM pip.track_state "
        "WHERE session_id = $1::uuid AND section_gid = $2",
        pqxx::params{session_uuid_, section});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<int>(), 0);
}

TEST_F(PipTrackStateFixture, Upsert_ConflictUpdatesTrains)
{
    const std::string section = "OT-zwr1";
    const std::string trains_v1 =
        R"([{"number":"IC 101","has_extra_info":false,"manually_placed":false,"entry_side":"LEFT"}])";
    const std::string trains_v2 = "[]";

    // First upsert: occupied
    writer_->upsert_pip_track_state(session_uuid_, section, trains_v1);

    // Second upsert: now free — ON CONFLICT DO UPDATE must overwrite.
    writer_->upsert_pip_track_state(session_uuid_, section, trains_v2);

    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT jsonb_array_length(trains) FROM pip.track_state "
        "WHERE session_id = $1::uuid AND section_gid = $2",
        pqxx::params{session_uuid_, section});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_EQ(r[0][0].as<int>(), 0) << "trains should have been updated to empty array";
}

TEST_F(PipTrackStateFixture, Upsert_MultipleSections_AllInserted)
{
    writer_->upsert_pip_track_state(session_uuid_, "OT-sec-1", "[]");
    writer_->upsert_pip_track_state(session_uuid_, "OT-sec-2", "[]");
    writer_->upsert_pip_track_state(session_uuid_, "OT-sec-3", "[]");

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
    const std::string section = "OT-tor_c";

    writer_->upsert_pip_track_state(session_uuid_, section, "[]");

    // Fetch original updated_at
    std::string first_ts;
    {
        pqxx::connection c{conn_str_};
        pqxx::work tx{c};
        const auto r = tx.exec(
            "SELECT updated_at FROM pip.track_state "
            "WHERE session_id = $1::uuid AND section_gid = $2",
            pqxx::params{session_uuid_, section});
        tx.commit();
        ASSERT_EQ(r.size(), 1u);
        first_ts = r[0][0].as<std::string>();
    }

    writer_->upsert_pip_track_state(session_uuid_, section, "[]");

    // updated_at must not be null after the second upsert.
    pqxx::connection c{conn_str_};
    pqxx::work tx{c};
    const auto r = tx.exec(
        "SELECT updated_at IS NOT NULL FROM pip.track_state "
        "WHERE session_id = $1::uuid AND section_gid = $2",
        pqxx::params{session_uuid_, section});
    tx.commit();

    ASSERT_EQ(r.size(), 1u);
    EXPECT_TRUE(r[0][0].as<bool>());
}
