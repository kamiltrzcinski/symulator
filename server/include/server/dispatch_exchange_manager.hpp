#pragma once

// server/include/server/dispatch_exchange_manager.hpp
//
// Manages the S-form dispatch-exchange state machine for each directed
// (src_dispatch_area, dst_dispatch_area) pair.
//
// Pure business logic — no I/O, no engine state, no threading.
// Must be driven from a single logical thread (DISPATCHER).
//
// State machine and S-form catalogue: docs/15-dispatch-forms.md

#include "engine/core/types.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <utility>

namespace server
{

using engine::core::DispatchFormType;
using engine::core::ExchangeStatus;
using engine::core::TelegramDirection;
using engine::core::TelegramStatus;

enum class TelegramResult : std::uint8_t
{
    ACCEPTED,
    REJECTED_WRONG_STATE,   // form not allowed in the current ExchangeStatus
    REJECTED_DUPLICATE,     // confirmation already received (e.g. second S24)
};

struct TelegramOutcome
{
    TelegramResult result;
    ExchangeStatus new_status;      // state after this telegram; unchanged on rejection
    TelegramStatus telegram_status; // mirrors result for the telegram row
    std::string    exchange_id;     // non-empty only when result == ACCEPTED
};

// Manages S-form exchange state per (src_area_id, dst_area_id) pair.
class DispatchExchangeManager
{
public:
    // Submit a dispatch telegram and advance the state machine.
    // src_area / dst_area are DispatchAreaID.value strings.
    [[nodiscard]] TelegramOutcome submit_telegram(
        const std::string& src_area,
        const std::string& dst_area,
        DispatchFormType   form,
        TelegramDirection  direction,
        const std::string& train_number);

    // Query the current ExchangeStatus for a directed pair.
    // Returns IDLE when no exchange has been started.
    [[nodiscard]] ExchangeStatus status(
        const std::string& src_area,
        const std::string& dst_area) const noexcept;

    // Advance a S26_RECEIVED exchange to CLOSED (arrival confirmed, EDR updated).
    // No-op for any other status.
    void close(const std::string& src_area, const std::string& dst_area);

private:
    struct ExchangeState
    {
        std::string    exchange_id;
        ExchangeStatus status       = ExchangeStatus::IDLE;
        std::string    train_number;
    };

    using Key = std::pair<std::string, std::string>;

    ExchangeState& get_or_create(const Key& key);
    std::string    generate_exchange_id();

    std::map<Key, ExchangeState> exchanges_;
    std::uint64_t                next_id_ = 1;
};

}  // namespace server
