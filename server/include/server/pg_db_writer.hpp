// server/include/server/pg_db_writer.hpp
//
// PostgreSQL implementation of IDbWriter.
// Uses libpqxx 8.x to talk to the PostgreSQL server defined in the connection string.
//
// Thread safety: pqxx::connection is NOT thread-safe.  All methods acquire
// mu_ before touching conn_.  For the current single-session server this is
// sufficient; a future version can add a connection pool.

#pragma once

#include "server/db_writer.hpp"

#include <pqxx/pqxx>

#include <mutex>
#include <string>

namespace server
{

class PgDbWriter : public IDbWriter
{
public:
    /// Connect to PostgreSQL using a libpq connection string.
    /// Format: "host=... port=... dbname=... user=... password=..."
    /// Throws std::runtime_error on connection failure.
    explicit PgDbWriter(const std::string& connection_string);

    /// INSERT INTO session.sessions, return the generated UUID as a string.
    /// Stores the UUID internally; subsequent write methods use it.
    std::string init_session(const std::string& display_name, int schema_version) override;

    /// INSERT INTO session.dispatch_telegrams.
    void write_dispatch_telegram(const std::string& session_id, TelegramRow row) override;

    /// UPDATE session.edr_entries SET track_clear_time = <time-of-day interval>.
    void update_edr_track_clear_time(const std::string& session_id, const std::string& train_number,
                                     const std::string& station_sid,
                                     std::uint64_t timestamp_us) override;

private:
    pqxx::connection conn_;
    std::string session_uuid_;
    std::mutex mu_;
};

}  // namespace server
