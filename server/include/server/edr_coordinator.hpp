#pragma once

// server/include/server/edr_coordinator.hpp
//
// EdrCoordinator tracks train arrival/departure times in session.edr_entries
// based on accepted S-form dispatch telegrams.
//
// Responsibilities:
//   S25 accepted  → set actual_departure + status=DEPARTED for the departing station.
//   S26 accepted  → set actual_arrival  + status=ARRIVED  for the arrival station.
//   S24/S56       → track_clear_time is handled by BilateralChannel directly via IDbWriter.
//
// Station resolution rule (SENT vs RECEIVED):
//   SENT:     the operator at src_area just sent the form  → affected station = src_area.
//   RECEIVED: the operator at src_area recorded a received form → affected station = dst_area.
//
// This is consistent regardless of whether the same server models one or both
// endpoints of the bilateral exchange.
//
// Threading: called from the IO thread (same thread as BilateralChannel callbacks).

#include "engine/core/types.hpp"
#include "server/db_writer.hpp"

#include <cstdint>
#include <string>

namespace server
{

class EdrCoordinator
{
public:
    EdrCoordinator(IDbWriter& db, std::string session_id);

    /// Called by BilateralChannel after a dispatch S-form is accepted.
    ///
    /// @param form          The accepted form type.
    /// @param direction     SENT = src_area is the affected station;
    ///                      RECEIVED = dst_area is the affected station.
    /// @param src_area      Dispatch area ID of the telegram's src_area_id field.
    /// @param dst_area      Dispatch area ID of the telegram's dst_area_id field.
    /// @param train_number  Train number from the telegram payload.
    /// @param timestamp_us  Acceptance timestamp (microseconds, system clock).
    void on_telegram_accepted(engine::core::DispatchFormType form,
                              engine::core::TelegramDirection direction,
                              const std::string& src_area, const std::string& dst_area,
                              const std::string& train_number, std::uint64_t timestamp_us);

private:
    IDbWriter& db_;
    std::string session_id_;
};

}  // namespace server
