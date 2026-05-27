#pragma once

// server/include/server/db_writer.hpp
//
// Abstract interface for writing session-scoped data to the database.
// A production impl talks to PostgreSQL; NullDbWriter is used in tests.

#include "engine/core/types.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace server
{

using engine::core::DispatchFormType;
using engine::core::ExchangeStatus;
using engine::core::TelegramDirection;

/// One row in session.dispatch_telegrams.
struct TelegramRow
{
    std::string form_type;    // "S2" | "S24" | ... | "FREE_TEXT"
    std::string exchange_id;  // e.g. "exch-0000001"
    std::string train_number;
    std::string from_sid;   // source dispatch area id
    std::string to_sid;     // destination dispatch area id
    std::string direction;  // "SENT" | "RECEIVED"
    std::string status;     // "PENDING" | "ACCEPTED" | "REJECTED" | "CLOSED"
    std::optional<std::string> track_number;
    std::vector<std::string> km_markers;
    std::string body;  // JSON snapshot of the payload
    std::uint64_t timestamp_us{0};
};

/// One row in session.events (append-only domain event log).
struct DomainEventRow
{
    uint8_t event_type{0};                  ///< DOMAIN_EVENT event_type byte (docs/09)
    uint32_t event_id{0};                   ///< server monotonic counter (same value as wire frame)
    uint64_t timestamp_us{0};               ///< microseconds since session epoch
    std::optional<std::string> object_gid;  ///< NULL for session-level events
    std::vector<uint8_t> payload;           ///< raw FlatBuffers body (without the 13-byte prefix)
};

class IDbWriter
{
public:
    virtual ~IDbWriter() = default;

    /// Create a session row in session.sessions and return its UUID string.
    /// Called once at server startup after the scenario is loaded.
    /// The returned UUID is used as the session_id for all subsequent DB writes.
    virtual std::string init_session(const std::string& display_name, int schema_version) = 0;

    /// Append one domain event to session.events.
    /// Called from the ENGINE thread for every DeviceStateChange that produces a
    /// DOMAIN_EVENT frame; the event_id and payload must match the wire frame.
    virtual void write_domain_event(const std::string& session_id, DomainEventRow row) = 0;

    /// Persist a dispatch telegram row.
    virtual void write_dispatch_telegram(const std::string& session_id, TelegramRow row) = 0;

    /// Update track_clear_time in session.edr_entries for the given train / station.
    /// Called when an S24 or S56 telegram is accepted.
    virtual void update_edr_track_clear_time(const std::string& session_id,
                                             const std::string& train_number,
                                             const std::string& station_sid,
                                             std::uint64_t timestamp_us) = 0;

    /// Set actual_departure and status=DEPARTED in session.edr_entries.
    /// Called when an S25 (departure notification) telegram is accepted.
    virtual void update_edr_departure(const std::string& session_id,
                                      const std::string& train_number,
                                      const std::string& station_sid,
                                      std::uint64_t timestamp_us) = 0;

    /// Set actual_arrival and status=ARRIVED in session.edr_entries.
    /// Called when an S26 (arrival confirmation) telegram is accepted.
    virtual void update_edr_arrival(const std::string& session_id, const std::string& train_number,
                                    const std::string& station_sid, std::uint64_t timestamp_us) = 0;

    /// UPSERT pip.track_state for one track section.
    /// trains_json is a JSON array string, e.g. "[]" or
    /// "[{\"number\":\"IC123\",\"has_extra_info\":false,\"manually_placed\":false,\"entry_side\":\"LEFT\"}]".
    /// path_confirmed is NOT touched — it is managed by route-confirmation commands.
    virtual void upsert_pip_track_state(const std::string& session_id,
                                        const std::string& section_gid,
                                        const std::string& trains_json) = 0;

    /// Persist a full session snapshot to session.snapshots.
    /// payload is the raw FlatBuffers bytes of the snapshot.
    virtual void save_snapshot(const std::string& session_id, int64_t seq_cursor,
                               int64_t timestamp_us, const std::vector<std::uint8_t>& payload) = 0;

    /// Append one message to session.chat_log.
    /// target_type is "BROADCAST" | "AREA" | "PLAYER"; target_id may be nullopt for broadcasts.
    virtual void append_chat_message(const std::string& session_id, const std::string& sender_id,
                                     const std::string& target_type,
                                     const std::optional<std::string>& target_id,
                                     const std::string& body, int64_t timestamp_us) = 0;

    /// Insert an operating-point assignment and return its auto-generated id.
    /// Records that client_id has taken operating_point_id at station_sid in this session.
    virtual int64_t assign_operating_point(const std::string& session_id,
                                           const std::string& operating_point_id,
                                           const std::string& station_sid,
                                           const std::string& client_id) = 0;

    /// Mark an operating-point assignment as released (set released_at = now()).
    /// Idempotent — no-op if the assignment is already released.
    virtual void release_operating_point(const std::string& session_id,
                                         const std::string& operating_point_id,
                                         const std::string& client_id) = 0;

    /// UPSERT a timetable template row in fleet.timetable_templates.
    /// scheduled_arrival_secs and track_number may be null for origin/terminus stations.
    virtual void upsert_timetable_template(const std::string& train_number,
                                           const std::string& station_sid,
                                           const std::optional<std::string>& operating_point_id,
                                           const std::optional<std::string>& scheduled_arrival_secs,
                                           const std::string& scheduled_departure_secs,
                                           const std::optional<std::string>& track_number,
                                           const std::string& stop_type) = 0;
};

/// No-op implementation for unit tests.
/// Captures all calls into public vectors for assertion.
class NullDbWriter : public IDbWriter
{
public:
    std::string init_session(const std::string& /*display_name*/, int /*schema_version*/) override
    {
        return "00000000-0000-0000-0000-000000000001";
    }

    void write_dispatch_telegram(const std::string& /*session_id*/, TelegramRow row) override
    {
        written_telegrams.push_back(std::move(row));
    }

    void write_domain_event(const std::string& /*session_id*/, DomainEventRow row) override
    {
        written_events.push_back(std::move(row));
    }

    void update_edr_track_clear_time(const std::string& /*session_id*/,
                                     const std::string& train_number,
                                     const std::string& station_sid,
                                     std::uint64_t timestamp_us) override
    {
        edr_updates.push_back({train_number, station_sid, timestamp_us});
    }

    void update_edr_departure(const std::string& /*session_id*/, const std::string& train_number,
                              const std::string& station_sid, std::uint64_t timestamp_us) override
    {
        edr_departures.push_back({train_number, station_sid, timestamp_us});
    }

    void update_edr_arrival(const std::string& /*session_id*/, const std::string& train_number,
                            const std::string& station_sid, std::uint64_t timestamp_us) override
    {
        edr_arrivals.push_back({train_number, station_sid, timestamp_us});
    }

    void upsert_pip_track_state(const std::string& /*session_id*/, const std::string& section_gid,
                                const std::string& trains_json) override
    {
        pip_upserts.push_back({section_gid, trains_json});
    }

    void save_snapshot(const std::string& /*session_id*/, int64_t seq_cursor, int64_t timestamp_us,
                       const std::vector<std::uint8_t>& payload) override
    {
        saved_snapshots.push_back({seq_cursor, timestamp_us, payload});
    }

    void append_chat_message(const std::string& /*session_id*/, const std::string& sender_id,
                             const std::string& target_type,
                             const std::optional<std::string>& target_id, const std::string& body,
                             int64_t timestamp_us) override
    {
        chat_messages.push_back({sender_id, target_type, target_id, body, timestamp_us});
    }

    int64_t assign_operating_point(const std::string& /*session_id*/,
                                   const std::string& operating_point_id,
                                   const std::string& station_sid,
                                   const std::string& client_id) override
    {
        op_assignments.push_back({operating_point_id, station_sid, client_id});
        return static_cast<int64_t>(op_assignments.size());
    }

    void release_operating_point(const std::string& /*session_id*/,
                                 const std::string& operating_point_id,
                                 const std::string& client_id) override
    {
        op_releases.push_back({operating_point_id, client_id});
    }

    void upsert_timetable_template(const std::string& train_number, const std::string& station_sid,
                                   const std::optional<std::string>& operating_point_id,
                                   const std::optional<std::string>& scheduled_arrival_secs,
                                   const std::string& scheduled_departure_secs,
                                   const std::optional<std::string>& track_number,
                                   const std::string& stop_type) override
    {
        timetable_upserts.push_back({train_number, station_sid, operating_point_id,
                                     scheduled_arrival_secs, scheduled_departure_secs, track_number,
                                     stop_type});
    }

    struct PipUpsert
    {
        std::string section_gid;
        std::string trains_json;
    };

    struct EdrUpdate
    {
        std::string train_number;
        std::string station_sid;
        std::uint64_t timestamp_us;
    };

    struct SnapshotSave
    {
        int64_t seq_cursor;
        int64_t timestamp_us;
        std::vector<std::uint8_t> payload;
    };

    struct ChatMessage
    {
        std::string sender_id;
        std::string target_type;
        std::optional<std::string> target_id;
        std::string body;
        int64_t timestamp_us;
    };

    struct OpAssignment
    {
        std::string operating_point_id;
        std::string station_sid;
        std::string client_id;
    };

    struct OpRelease
    {
        std::string operating_point_id;
        std::string client_id;
    };

    struct TimetableUpsert
    {
        std::string train_number;
        std::string station_sid;
        std::optional<std::string> operating_point_id;
        std::optional<std::string> scheduled_arrival_secs;
        std::string scheduled_departure_secs;
        std::optional<std::string> track_number;
        std::string stop_type;
    };

    std::vector<TelegramRow> written_telegrams;
    std::vector<DomainEventRow> written_events;
    std::vector<EdrUpdate> edr_updates;
    std::vector<EdrUpdate> edr_departures;
    std::vector<EdrUpdate> edr_arrivals;
    std::vector<PipUpsert> pip_upserts;
    std::vector<SnapshotSave> saved_snapshots;
    std::vector<ChatMessage> chat_messages;
    std::vector<OpAssignment> op_assignments;
    std::vector<OpRelease> op_releases;
    std::vector<TimetableUpsert> timetable_upserts;
};

}  // namespace server
