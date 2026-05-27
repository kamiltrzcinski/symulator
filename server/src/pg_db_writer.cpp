// server/src/pg_db_writer.cpp

#include "server/pg_db_writer.hpp"

#include <stdexcept>
#include <string>

namespace server
{

// ── Constructor ───────────────────────────────────────────────────────────────

PgDbWriter::PgDbWriter(const std::string& connection_string) : conn_{connection_string} {}

// ── init_session ──────────────────────────────────────────────────────────────

std::string PgDbWriter::init_session(const std::string& display_name, int schema_version)
{
    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    const auto r = tx.exec(
        "INSERT INTO session.sessions (display_name, schema_version, status, started_at) "
        "VALUES ($1, $2, 'STARTED', now()) RETURNING id::text",
        pqxx::params{display_name, schema_version});
    tx.commit();

    if (r.empty() || r[0][0].is_null())
        throw std::runtime_error("[PgDbWriter] init_session: no UUID returned");

    session_uuid_ = r[0][0].as<std::string>();
    return session_uuid_;
}

// ── write_domain_event ────────────────────────────────────────────────────────

void PgDbWriter::write_domain_event(const std::string& /*session_id*/, DomainEventRow row)
{
    const pqxx::bytes_view payload_bv{reinterpret_cast<const std::byte*>(row.payload.data()),
                                      row.payload.size()};

    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    tx.exec(
        "INSERT INTO session.events "
        "  (session_id, event_type, event_id, timestamp_us, object_gid, payload) "
        "VALUES ($1::uuid, $2, $3, $4, $5, $6)",
        pqxx::params{session_uuid_, static_cast<int>(row.event_type),
                     static_cast<int64_t>(row.event_id), static_cast<int64_t>(row.timestamp_us),
                     row.object_gid,  // std::optional<std::string>: NULL when empty
                     payload_bv});
    tx.commit();
}

// ── write_dispatch_telegram ───────────────────────────────────────────────────

void PgDbWriter::write_dispatch_telegram(const std::string& /*session_id*/, TelegramRow row)
{
    // Build a PostgreSQL TEXT[] literal: {"val1","val2",...}
    std::string km_array = "{";
    for (std::size_t i = 0; i < row.km_markers.size(); ++i)
    {
        if (i > 0)
            km_array += ',';
        km_array += '"';
        for (char c : row.km_markers[i])
        {
            if (c == '"' || c == '\\')
                km_array += '\\';
            km_array += c;
        }
        km_array += '"';
    }
    km_array += '}';

    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    tx.exec(
        "INSERT INTO session.dispatch_telegrams "
        "  (session_id, form_type, exchange_id, train_number, from_sid, to_sid, "
        "   direction, status, track_number, km_markers, body, timestamp_us) "
        "VALUES ($1::uuid, $2, $3, $4, $5, $6, $7, $8, $9, $10::text[], $11, $12)",
        pqxx::params{session_uuid_, row.form_type, row.exchange_id, row.train_number, row.from_sid,
                     row.to_sid, row.direction, row.status,
                     row.track_number,  // std::optional<std::string>: NULL when empty
                     km_array, row.body, static_cast<int64_t>(row.timestamp_us)});
    tx.commit();
}

// ── update_edr_track_clear_time ───────────────────────────────────────────────

void PgDbWriter::update_edr_track_clear_time(const std::string& /*session_id*/,
                                             const std::string& train_number,
                                             const std::string& station_sid,
                                             std::uint64_t timestamp_us)
{
    // Convert absolute timestamp to time-of-day INTERVAL.
    // timestamp_us % 86400000000 gives microseconds since midnight.
    // make_interval(secs => N) accepts fractional seconds.
    static constexpr int64_t kUsPerDay = 86'400'000'000LL;

    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    tx.exec(
        "UPDATE session.edr_entries "
        "SET track_clear_time = make_interval(secs => "
        "    ($4::bigint % $5::bigint)::double precision / 1000000.0), "
        "    updated_at = now() "
        "WHERE session_id = $1::uuid "
        "  AND train_number = $2 "
        "  AND station_sid  = $3",
        pqxx::params{session_uuid_, train_number, station_sid, static_cast<int64_t>(timestamp_us),
                     kUsPerDay});
    tx.commit();
}

// ── update_edr_departure ──────────────────────────────────────────────────────

void PgDbWriter::update_edr_departure(const std::string& /*session_id*/,
                                      const std::string& train_number,
                                      const std::string& station_sid, std::uint64_t timestamp_us)
{
    static constexpr int64_t kUsPerDay = 86'400'000'000LL;

    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    tx.exec(
        "UPDATE session.edr_entries "
        "SET actual_departure = make_interval(secs => "
        "    ($4::bigint % $5::bigint)::double precision / 1000000.0), "
        "    status = 'DEPARTED', "
        "    updated_at = now() "
        "WHERE session_id = $1::uuid "
        "  AND train_number = $2 "
        "  AND station_sid  = $3 "
        "  AND status != 'DEPARTED'",
        pqxx::params{session_uuid_, train_number, station_sid, static_cast<int64_t>(timestamp_us),
                     kUsPerDay});
    tx.commit();
}

// ── update_edr_arrival ────────────────────────────────────────────────────────

void PgDbWriter::update_edr_arrival(const std::string& /*session_id*/,
                                    const std::string& train_number, const std::string& station_sid,
                                    std::uint64_t timestamp_us)
{
    static constexpr int64_t kUsPerDay = 86'400'000'000LL;

    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    tx.exec(
        "UPDATE session.edr_entries "
        "SET actual_arrival = make_interval(secs => "
        "    ($4::bigint % $5::bigint)::double precision / 1000000.0), "
        "    status = 'ARRIVED', "
        "    updated_at = now() "
        "WHERE session_id = $1::uuid "
        "  AND train_number = $2 "
        "  AND station_sid  = $3 "
        "  AND status NOT IN ('ARRIVED', 'DEPARTED')",
        pqxx::params{session_uuid_, train_number, station_sid, static_cast<int64_t>(timestamp_us),
                     kUsPerDay});
    tx.commit();
}

// ── upsert_pip_track_state ─────────────────────────────────────────────────────────────────────

void PgDbWriter::upsert_pip_track_state(const std::string& /*session_id*/,
                                        const std::string& section_gid,
                                        const std::string& trains_json)
{
    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    tx.exec(
        "INSERT INTO pip.track_state (session_id, section_gid, trains, updated_at) "
        "VALUES ($1::uuid, $2, $3::jsonb, now()) "
        "ON CONFLICT (session_id, section_gid) DO UPDATE "
        "  SET trains = EXCLUDED.trains, "
        "      updated_at = now()",
        pqxx::params{session_uuid_, section_gid, trains_json});
    tx.commit();
}

// ── save_snapshot ──────────────────────────────────────────────────────────────

void PgDbWriter::save_snapshot(const std::string& /*session_id*/, int64_t seq_cursor,
                               int64_t timestamp_us, const std::vector<std::uint8_t>& payload)
{
    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    const pqxx::bytes_view bv{reinterpret_cast<const std::byte*>(payload.data()), payload.size()};
    tx.exec(
        "INSERT INTO session.snapshots (session_id, seq_cursor, timestamp_us, payload) "
        "VALUES ($1::uuid, $2, $3, $4)",
        pqxx::params{session_uuid_, seq_cursor, timestamp_us, bv});
    tx.commit();
}

// ── append_chat_message ────────────────────────────────────────────────────────

void PgDbWriter::append_chat_message(const std::string& /*session_id*/,
                                     const std::string& sender_id, const std::string& target_type,
                                     const std::optional<std::string>& target_id,
                                     const std::string& body, int64_t timestamp_us)
{
    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    tx.exec(
        "INSERT INTO session.chat_log "
        "  (session_id, sender_id, target_type, target_id, body, timestamp_us) "
        "VALUES ($1::uuid, $2, $3, $4, $5, $6)",
        pqxx::params{session_uuid_, sender_id, target_type, target_id, body, timestamp_us});
    tx.commit();
}

// ── assign_operating_point ────────────────────────────────────────────────────

int64_t PgDbWriter::assign_operating_point(const std::string& /*session_id*/,
                                           const std::string& operating_point_id,
                                           const std::string& station_sid,
                                           const std::string& client_id)
{
    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    const auto result = tx.exec(
        "INSERT INTO session.operating_point_assignments "
        "  (session_id, operating_point_id, station_sid, client_id) "
        "VALUES ($1::uuid, $2, $3, $4) "
        "RETURNING id",
        pqxx::params{session_uuid_, operating_point_id, station_sid, client_id});
    tx.commit();
    return result.at(0, 0).as<int64_t>();
}

// ── release_operating_point ───────────────────────────────────────────────────

void PgDbWriter::release_operating_point(const std::string& /*session_id*/,
                                         const std::string& operating_point_id,
                                         const std::string& client_id)
{
    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    tx.exec(
        "UPDATE session.operating_point_assignments "
        "SET released_at = now() "
        "WHERE session_id = $1::uuid "
        "  AND operating_point_id = $2 "
        "  AND client_id = $3 "
        "  AND released_at IS NULL",
        pqxx::params{session_uuid_, operating_point_id, client_id});
    tx.commit();
}

// ── upsert_timetable_template ─────────────────────────────────────────────────

void PgDbWriter::upsert_timetable_template(const std::string& train_number,
                                           const std::string& station_sid,
                                           const std::optional<std::string>& operating_point_id,
                                           const std::optional<std::string>& scheduled_arrival_secs,
                                           const std::string& scheduled_departure_secs,
                                           const std::optional<std::string>& track_number,
                                           const std::string& stop_type)
{
    std::lock_guard<std::mutex> lock{mu_};

    pqxx::work tx{conn_};
    tx.exec(
        "INSERT INTO fleet.timetable_templates "
        "  (train_number, station_sid, operating_point_id, "
        "   scheduled_arrival, scheduled_departure, track_number, stop_type) "
        "VALUES ($1, $2, $3, "
        "        CASE WHEN $4::text IS NULL THEN NULL "
        "             ELSE make_interval(secs => $4::double precision) END, "
        "        make_interval(secs => $5::double precision), "
        "        $6, $7) "
        "ON CONFLICT (train_number, station_sid) DO UPDATE "
        "  SET operating_point_id        = EXCLUDED.operating_point_id, "
        "      scheduled_arrival         = EXCLUDED.scheduled_arrival, "
        "      scheduled_departure       = EXCLUDED.scheduled_departure, "
        "      track_number              = EXCLUDED.track_number, "
        "      stop_type                 = EXCLUDED.stop_type",
        pqxx::params{train_number, station_sid, operating_point_id, scheduled_arrival_secs,
                     scheduled_departure_secs, track_number, stop_type});
    tx.commit();
}

}  // namespace server
