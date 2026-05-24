// server/src/edr_coordinator.cpp

#include "server/edr_coordinator.hpp"

namespace server
{

EdrCoordinator::EdrCoordinator(IDbWriter& db, std::string session_id)
    : db_{db}, session_id_{std::move(session_id)}
{
}

void EdrCoordinator::on_telegram_accepted(engine::core::DispatchFormType form,
                                          engine::core::TelegramDirection direction,
                                          const std::string& src_area, const std::string& dst_area,
                                          const std::string& train_number,
                                          std::uint64_t timestamp_us)
{
    // SENT: the operator at src_area sent the form → src_area is the affected station.
    // RECEIVED: the operator at src_area recorded a received form → dst_area is the origin.
    const std::string& station =
        (direction == engine::core::TelegramDirection::SENT) ? src_area : dst_area;

    switch (form)
    {
        case engine::core::DispatchFormType::S25:
            // Train departed from the affected station.
            db_.update_edr_departure(session_id_, train_number, station, timestamp_us);
            break;

        case engine::core::DispatchFormType::S26:
            // Train arrived at the affected station.
            db_.update_edr_arrival(session_id_, train_number, station, timestamp_us);
            break;

        default:
            // S2, S24, S55, S56 — not handled here.
            // S24/S56 track_clear_time is written by BilateralChannel directly.
            break;
    }
}

}  // namespace server
