// server/src/dispatch_coordinator.cpp

#include "server/dispatch_coordinator.hpp"

#include <string>

namespace server
{

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::string form_type_str(engine::core::DispatchFormType ft) noexcept
{
    switch (ft)
    {
        case engine::core::DispatchFormType::S2:
            return "S2";
        case engine::core::DispatchFormType::S24:
            return "S24";
        case engine::core::DispatchFormType::S25:
            return "S25";
        case engine::core::DispatchFormType::S26:
            return "S26";
        case engine::core::DispatchFormType::S35:
            return "S35";
        case engine::core::DispatchFormType::S51:
            return "S51";
        case engine::core::DispatchFormType::S52:
            return "S52";
        case engine::core::DispatchFormType::S55:
            return "S55";
        case engine::core::DispatchFormType::S56:
            return "S56";
        case engine::core::DispatchFormType::S76:
            return "S76";
    }
    return "UNKNOWN";
}

// ── DispatchCoordinator ───────────────────────────────────────────────────────

DispatchCoordinator::DispatchCoordinator(DispatchExchangeManager& exchanges, IDbWriter& db_writer,
                                         EdrCoordinator& edr, std::string session_id)
    : exchanges_{exchanges}, db_writer_{db_writer}, edr_{edr}, session_id_{std::move(session_id)}
{
}

std::optional<DispatchCoordinator::DispatchOutcome> DispatchCoordinator::handle_dispatch_form(
    engine::core::DispatchFormType form, engine::core::TelegramDirection direction,
    const std::string& src_area, const std::string& dst_area, const std::string& train_number,
    const std::optional<std::string>& track_number, const std::vector<std::string>& km_markers,
    std::uint64_t timestamp_us)
{
    // 1. Drive the state machine.
    const auto outcome =
        exchanges_.submit_telegram(src_area, dst_area, form, direction, train_number);

    TelegramRow row;
    row.form_type = form_type_str(form);
    row.exchange_id = outcome.exchange_id.value_or("");
    row.train_number = train_number;
    row.from_uid = std::stoull(src_area);
    row.to_uid = std::stoull(dst_area);
    row.direction = (direction == engine::core::TelegramDirection::SENT) ? "SENT" : "RECEIVED";
    row.track_number = track_number;
    row.km_markers = km_markers;
    row.body = "";
    row.timestamp_us = timestamp_us;

    if (outcome.result != TelegramResult::ACCEPTED)
    {
        row.status = "REJECTED_STRICT_POLICY";
        db_writer_.write_dispatch_telegram(session_id_, row);
        return std::nullopt;
    }

    // 2. Persist the telegram row.
    row.status = "ACCEPTED";

    db_writer_.write_dispatch_telegram(session_id_, row);

    // 3. S24/S56 — update EDR track_clear_time.
    if (form == engine::core::DispatchFormType::S24 || form == engine::core::DispatchFormType::S56)
    {
        db_writer_.update_edr_track_clear_time(session_id_, train_number, std::stoull(dst_area),
                                               timestamp_us);
    }

    // 4. S25 (departure) / S26 (arrival) — update EDR via EdrCoordinator.
    edr_.on_telegram_accepted(form, direction, src_area, dst_area, train_number, timestamp_us);

    return DispatchOutcome{outcome.exchange_id, outcome.new_status};
}

void DispatchCoordinator::handle_free_text(const std::string& src_area, const std::string& dst_area,
                                           const std::string& body, std::uint64_t timestamp_us)
{
    TelegramRow row;
    row.form_type = "FREE_TEXT";
    row.exchange_id = "";  // free text has no exchange state
    row.train_number = "";
    row.from_uid = std::stoull(src_area);
    row.to_uid = std::stoull(dst_area);
    row.direction = "SENT";
    row.status = "ACCEPTED";
    row.body = body;
    row.timestamp_us = timestamp_us;

    db_writer_.write_dispatch_telegram(session_id_, std::move(row));
}

}  // namespace server
