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

class IDbWriter
{
public:
    virtual ~IDbWriter() = default;

    /// Persist a dispatch telegram row.
    virtual void write_dispatch_telegram(const std::string& session_id, TelegramRow row) = 0;

    /// Update track_clear_time in session.edr_entries for the given train / station.
    /// Called when an S24 or S56 telegram is accepted.
    virtual void update_edr_track_clear_time(const std::string& session_id,
                                             const std::string& train_number,
                                             const std::string& station_sid,
                                             std::uint64_t timestamp_us) = 0;
};

/// No-op implementation for unit tests.
/// Captures all calls into public vectors for assertion.
class NullDbWriter : public IDbWriter
{
public:
    void write_dispatch_telegram(const std::string& /*session_id*/, TelegramRow row) override
    {
        written_telegrams.push_back(std::move(row));
    }

    void update_edr_track_clear_time(const std::string& /*session_id*/,
                                     const std::string& train_number,
                                     const std::string& station_sid,
                                     std::uint64_t timestamp_us) override
    {
        edr_updates.push_back({train_number, station_sid, timestamp_us});
    }

    struct EdrUpdate
    {
        std::string train_number;
        std::string station_sid;
        std::uint64_t timestamp_us;
    };

    std::vector<TelegramRow> written_telegrams;
    std::vector<EdrUpdate> edr_updates;
};

}  // namespace server
