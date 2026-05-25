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

    std::vector<TelegramRow> written_telegrams;
    std::vector<DomainEventRow> written_events;
    std::vector<EdrUpdate> edr_updates;
    std::vector<EdrUpdate> edr_departures;
    std::vector<EdrUpdate> edr_arrivals;
    std::vector<PipUpsert> pip_upserts;
};

}  // namespace server
