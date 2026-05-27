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

    /// INSERT INTO session.events.
    void write_domain_event(const std::string& session_id, DomainEventRow row) override;

    /// INSERT INTO session.dispatch_telegrams.
    void write_dispatch_telegram(const std::string& session_id, TelegramRow row) override;

    /// UPDATE session.edr_entries SET track_clear_time = <time-of-day interval>.
    void update_edr_track_clear_time(const std::string& session_id, const std::string& train_number,
                                     const std::string& station_sid,
                                     std::uint64_t timestamp_us) override;

    /// UPDATE session.edr_entries SET actual_departure, status='DEPARTED'.
    void update_edr_departure(const std::string& session_id, const std::string& train_number,
                              const std::string& station_sid, std::uint64_t timestamp_us) override;

    /// UPDATE session.edr_entries SET actual_arrival, status='ARRIVED'.
    void update_edr_arrival(const std::string& session_id, const std::string& train_number,
                            const std::string& station_sid, std::uint64_t timestamp_us) override;

    /// UPSERT pip.track_state for one track section.
    void upsert_pip_track_state(const std::string& session_id, const std::string& section_gid,
                                const std::string& trains_json) override;

    /// INSERT INTO session.snapshots.
    void save_snapshot(const std::string& session_id, int64_t seq_cursor, int64_t timestamp_us,
                       const std::vector<std::uint8_t>& payload) override;

    /// INSERT INTO session.chat_log.
    void append_chat_message(const std::string& session_id, const std::string& sender_id,
                             const std::string& target_type,
                             const std::optional<std::string>& target_id, const std::string& body,
                             int64_t timestamp_us) override;

    /// INSERT INTO session.operating_point_assignments, RETURNING id.
    int64_t assign_operating_point(const std::string& session_id,
                                   const std::string& operating_point_id,
                                   const std::string& station_sid,
                                   const std::string& client_id) override;

    /// UPDATE session.operating_point_assignments SET released_at = now() WHERE ... AND released_at IS NULL.
    void release_operating_point(const std::string& session_id,
                                 const std::string& operating_point_id,
                                 const std::string& client_id) override;

    /// UPSERT fleet.timetable_templates.
    void upsert_timetable_template(const std::string& train_number, const std::string& station_sid,
                                   const std::optional<std::string>& operating_point_id,
                                   const std::optional<std::string>& scheduled_arrival_secs,
                                   const std::string& scheduled_departure_secs,
                                   const std::optional<std::string>& track_number,
                                   const std::string& stop_type) override;

private:
    pqxx::connection conn_;
    std::string session_uuid_;
    std::mutex mu_;
};

}  // namespace server
