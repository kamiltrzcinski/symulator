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
    std::string exchange_id;  // server-generated exchange correlation ID (e.g. "exch-0000001")
    std::string train_number;
    std::uint64_t from_uid{0};  // source dispatch area UID
    std::uint64_t to_uid{0};    // destination dispatch area UID
    std::string direction;      // "SENT" | "RECEIVED"
    std::string status;         // "PENDING" | "ACCEPTED" | "REJECTED" | "CLOSED"
    std::optional<std::string> track_number;
    std::vector<std::string> km_markers;
    std::string body;
    std::uint64_t timestamp_us{0};
};

/// One row in session.events (append-only domain event log).
struct DomainEventRow
{
    uint8_t event_type{0};
    uint32_t event_id{0};
    uint64_t timestamp_us{0};
    std::optional<std::uint64_t> object_uid;  ///< NULL for session-level events
    std::vector<uint8_t> payload;
};

/// One operator-visible EDR journal row.
struct EdrJournalEntryRow
{
    std::string operating_point_id;
    std::uint64_t station_uid{0};  // INFRASTRUCTURE/STATION UID
    std::string journal_page;
    std::string entry_type;
    std::optional<std::string> train_number;
    std::optional<std::string> direction;
    std::optional<std::string> track_number;
    std::optional<std::string> scheduled_arrival_secs;
    std::optional<std::string> scheduled_departure_secs;
    std::optional<std::string> actual_arrival_secs;
    std::optional<std::string> actual_departure_secs;
    std::string body;
    std::optional<std::string> notes;
    std::uint64_t timestamp_us{0};
};

class IDbWriter
{
public:
    virtual ~IDbWriter() = default;

    virtual std::string init_session(const std::string& display_name, int schema_version) = 0;

    virtual int64_t seed_edr_entries_for_operating_day(const std::string& session_id,
                                                       int iso_weekday) = 0;

    virtual void write_domain_event(const std::string& session_id, DomainEventRow row) = 0;

    virtual void write_dispatch_telegram(const std::string& session_id, TelegramRow row) = 0;

    virtual void update_edr_track_clear_time(const std::string& session_id,
                                             const std::string& train_number,
                                             std::uint64_t station_uid,
                                             std::uint64_t timestamp_us) = 0;

    virtual void update_edr_departure(const std::string& session_id,
                                      const std::string& train_number, std::uint64_t station_uid,
                                      std::uint64_t timestamp_us) = 0;

    virtual void update_edr_arrival(const std::string& session_id, const std::string& train_number,
                                    std::uint64_t station_uid, std::uint64_t timestamp_us) = 0;

    virtual int64_t append_edr_journal_entry(const std::string& session_id,
                                             EdrJournalEntryRow row) = 0;

    virtual void update_edr_journal_entry_status(const std::string& session_id, int64_t entry_id,
                                                 const std::string& status,
                                                 const std::optional<std::string>& notes) = 0;

    virtual void upsert_pip_track_state(const std::string& session_id, std::uint64_t section_uid,
                                        const std::string& trains_json) = 0;

    virtual void save_snapshot(const std::string& session_id, int64_t seq_cursor,
                               int64_t timestamp_us, const std::vector<std::uint8_t>& payload) = 0;

    virtual void append_chat_message(const std::string& session_id, const std::string& sender_id,
                                     const std::string& target_type,
                                     const std::optional<std::string>& target_id,
                                     const std::string& body, int64_t timestamp_us) = 0;

    virtual int64_t assign_operating_point(const std::string& session_id,
                                           const std::string& operating_point_id,
                                           const std::string& station_sid,
                                           const std::string& client_id) = 0;

    virtual void release_operating_point(const std::string& session_id,
                                         const std::string& operating_point_id,
                                         const std::string& client_id) = 0;

    virtual void upsert_timetable_template(const std::string& train_number,
                                           std::uint64_t station_uid,
                                           const std::optional<std::string>& operating_point_id,
                                           const std::optional<std::string>& scheduled_arrival_secs,
                                           const std::string& scheduled_departure_secs,
                                           const std::optional<std::string>& track_number,
                                           const std::string& stop_type,
                                           const std::vector<int>& operating_days) = 0;
};

/// No-op implementation for unit tests.
class NullDbWriter : public IDbWriter
{
public:
    std::string init_session(const std::string& /*display_name*/, int /*schema_version*/) override
    {
        return "00000000-0000-0000-0000-000000000001";
    }

    int64_t seed_edr_entries_for_operating_day(const std::string& /*session_id*/,
                                               int iso_weekday) override
    {
        seeded_operating_days.push_back(iso_weekday);
        return 0;
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
                                     const std::string& train_number, std::uint64_t station_uid,
                                     std::uint64_t timestamp_us) override
    {
        edr_updates.push_back({train_number, station_uid, timestamp_us});
    }

    void update_edr_departure(const std::string& /*session_id*/, const std::string& train_number,
                              std::uint64_t station_uid, std::uint64_t timestamp_us) override
    {
        edr_departures.push_back({train_number, station_uid, timestamp_us});
    }

    void update_edr_arrival(const std::string& /*session_id*/, const std::string& train_number,
                            std::uint64_t station_uid, std::uint64_t timestamp_us) override
    {
        edr_arrivals.push_back({train_number, station_uid, timestamp_us});
    }

    int64_t append_edr_journal_entry(const std::string& /*session_id*/,
                                     EdrJournalEntryRow row) override
    {
        edr_journal_entries.push_back(std::move(row));
        return static_cast<int64_t>(edr_journal_entries.size());
    }

    void update_edr_journal_entry_status(const std::string& /*session_id*/, int64_t entry_id,
                                         const std::string& status,
                                         const std::optional<std::string>& notes) override
    {
        edr_journal_status_updates.push_back({entry_id, status, notes});
    }

    void upsert_pip_track_state(const std::string& /*session_id*/, std::uint64_t section_uid,
                                const std::string& trains_json) override
    {
        pip_upserts.push_back({section_uid, trains_json});
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

    void upsert_timetable_template(const std::string& train_number, std::uint64_t station_uid,
                                   const std::optional<std::string>& operating_point_id,
                                   const std::optional<std::string>& scheduled_arrival_secs,
                                   const std::string& scheduled_departure_secs,
                                   const std::optional<std::string>& track_number,
                                   const std::string& stop_type,
                                   const std::vector<int>& operating_days) override
    {
        timetable_upserts.push_back({train_number, station_uid, operating_point_id,
                                     scheduled_arrival_secs, scheduled_departure_secs, track_number,
                                     stop_type, operating_days});
    }

    struct PipUpsert
    {
        std::uint64_t section_uid;
        std::string trains_json;
    };

    struct EdrUpdate
    {
        std::string train_number;
        std::uint64_t station_uid;
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
        std::uint64_t station_uid;
        std::optional<std::string> operating_point_id;
        std::optional<std::string> scheduled_arrival_secs;
        std::string scheduled_departure_secs;
        std::optional<std::string> track_number;
        std::string stop_type;
        std::vector<int> operating_days;
    };

    struct EdrJournalStatusUpdate
    {
        int64_t entry_id;
        std::string status;
        std::optional<std::string> notes;
    };

    std::vector<int> seeded_operating_days;
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
    std::vector<EdrJournalEntryRow> edr_journal_entries;
    std::vector<EdrJournalStatusUpdate> edr_journal_status_updates;
};

}  // namespace server
