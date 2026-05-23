// server/src/dispatch_exchange_manager.cpp

#include "server/dispatch_exchange_manager.hpp"

#include <format>

namespace server
{

// ── Helpers ───────────────────────────────────────────────────────────────────

static TelegramStatus to_telegram_status(TelegramResult r) noexcept
{
    switch (r)
    {
        case TelegramResult::ACCEPTED:            return TelegramStatus::CONFIRMED;
        case TelegramResult::REJECTED_WRONG_STATE:
        case TelegramResult::REJECTED_DUPLICATE:  return TelegramStatus::REJECTED;
    }
    return TelegramStatus::REJECTED;
}

// ── DispatchExchangeManager ───────────────────────────────────────────────────

std::string DispatchExchangeManager::generate_exchange_id()
{
    return std::format("exch-{:07}", next_id_++);
}

DispatchExchangeManager::ExchangeState&
DispatchExchangeManager::get_or_create(const Key& key)
{
    return exchanges_[key];
}

TelegramOutcome DispatchExchangeManager::submit_telegram(
    const std::string& src_area,
    const std::string& dst_area,
    DispatchFormType   form,
    TelegramDirection  direction,
    const std::string& train_number)
{
    const Key key{src_area, dst_area};
    ExchangeState& ex = get_or_create(key);

    auto reject = [&](TelegramResult reason) -> TelegramOutcome {
        return {reason, ex.status, to_telegram_status(reason), {}};
    };

    auto accept = [&](ExchangeStatus new_status) -> TelegramOutcome {
        ex.status       = new_status;
        ex.train_number = train_number;
        return {TelegramResult::ACCEPTED, new_status, TelegramStatus::CONFIRMED, ex.exchange_id};
    };

    // ── S2 / S55 — dispatch request ─────────────────────────────────────────
    if ((form == DispatchFormType::S2 || form == DispatchFormType::S55) &&
        direction == TelegramDirection::SENT)
    {
        switch (ex.status)
        {
            case ExchangeStatus::IDLE:
            case ExchangeStatus::CLOSED:
            case ExchangeStatus::CANCELLED:
                ex.exchange_id = generate_exchange_id();
                return accept(ExchangeStatus::S2_SENT);
            case ExchangeStatus::S2_SENT:
                return reject(TelegramResult::REJECTED_DUPLICATE);
            default:
                return reject(TelegramResult::REJECTED_WRONG_STATE);
        }
    }

    // ── S24 / S56 — line-clear reply ─────────────────────────────────────────
    if ((form == DispatchFormType::S24 || form == DispatchFormType::S56) &&
        direction == TelegramDirection::RECEIVED)
    {
        switch (ex.status)
        {
            case ExchangeStatus::S2_SENT:
                return accept(ExchangeStatus::S24_RECEIVED);
            case ExchangeStatus::S24_RECEIVED:
                return reject(TelegramResult::REJECTED_DUPLICATE);
            default:
                return reject(TelegramResult::REJECTED_WRONG_STATE);
        }
    }

    // ── S25 — departure notification ─────────────────────────────────────────
    if (form == DispatchFormType::S25 && direction == TelegramDirection::SENT)
    {
        if (ex.status == ExchangeStatus::S24_RECEIVED)
            return accept(ExchangeStatus::S25_SENT);
        return reject(TelegramResult::REJECTED_WRONG_STATE);
    }

    // ── S26 — arrival confirmation ────────────────────────────────────────────
    if (form == DispatchFormType::S26 && direction == TelegramDirection::RECEIVED)
    {
        if (ex.status == ExchangeStatus::S25_SENT)
            return accept(ExchangeStatus::S26_RECEIVED);
        return reject(TelegramResult::REJECTED_WRONG_STATE);
    }

    // ── S35 — cancellation request ────────────────────────────────────────────
    if (form == DispatchFormType::S35 && direction == TelegramDirection::SENT)
    {
        if (ex.status == ExchangeStatus::S2_SENT)
            return accept(ExchangeStatus::CANCELLED);
        return reject(TelegramResult::REJECTED_WRONG_STATE);
    }

    // ── S51 / S52 — level-crossing notifications (supplementary) ─────────────
    // These do not advance the main exchange status; they are accepted whenever
    // an exchange is active (S2_SENT … S25_SENT) or for S51 even from IDLE.
    if (form == DispatchFormType::S51 || form == DispatchFormType::S52)
    {
        const bool active = (ex.status != ExchangeStatus::IDLE &&
                             ex.status != ExchangeStatus::CLOSED &&
                             ex.status != ExchangeStatus::CANCELLED);
        if (!active)
            return reject(TelegramResult::REJECTED_WRONG_STATE);
        // No status transition — just record acceptance.
        return {TelegramResult::ACCEPTED, ex.status, TelegramStatus::CONFIRMED, ex.exchange_id};
    }

    // ── S76 — free-form message (accepted in any non-terminal state) ──────────
    if (form == DispatchFormType::S76)
    {
        const bool active = (ex.status != ExchangeStatus::CLOSED &&
                             ex.status != ExchangeStatus::CANCELLED);
        if (!active && ex.status != ExchangeStatus::IDLE)
            return reject(TelegramResult::REJECTED_WRONG_STATE);
        return {TelegramResult::ACCEPTED, ex.status, TelegramStatus::CONFIRMED, ex.exchange_id};
    }

    return reject(TelegramResult::REJECTED_WRONG_STATE);
}

ExchangeStatus DispatchExchangeManager::status(
    const std::string& src_area,
    const std::string& dst_area) const noexcept
{
    const Key key{src_area, dst_area};
    auto it = exchanges_.find(key);
    if (it == exchanges_.end())
        return ExchangeStatus::IDLE;
    return it->second.status;
}

void DispatchExchangeManager::close(
    const std::string& src_area,
    const std::string& dst_area)
{
    const Key key{src_area, dst_area};
    auto it = exchanges_.find(key);
    if (it == exchanges_.end())
        return;
    if (it->second.status == ExchangeStatus::S26_RECEIVED)
        it->second.status = ExchangeStatus::CLOSED;
}

}  // namespace server
