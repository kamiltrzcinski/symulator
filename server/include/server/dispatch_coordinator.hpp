#pragma once

// server/include/server/dispatch_coordinator.hpp
//
// Coordinates dispatch S-form business logic for msg_type 0x61 DISPATCH_CHANNEL_MESSAGE.
//
// Responsibilities:
//   1. Drive the DispatchExchangeManager state machine.
//   2. Persist accepted telegrams via IDbWriter.
//   3. Update EDR track_clear_time for S24/S56.
//   4. Notify EdrCoordinator for S25/S26 so EDR entries are updated.
//
// DispatchCoordinator operates purely on engine-domain types (no FlatBuffers / proto).
// Wire-format parsing and result serialisation are the responsibility of DispatchChannel.
//
// Threading: must be called from the IO thread (same thread as DispatchChannel callbacks).

#include "engine/core/types.hpp"
#include "server/db_writer.hpp"
#include "server/dispatch_exchange_manager.hpp"
#include "server/edr_coordinator.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace server
{

class DispatchCoordinator
{
public:
    DispatchCoordinator(DispatchExchangeManager& exchanges, IDbWriter& db_writer,
                        EdrCoordinator& edr, std::string session_id);

    /// Result of a successfully handled dispatch-form telegram.
    /// Returned to DispatchChannel so it can build the outbound wire frame.
    struct DispatchOutcome
    {
        std::string exchange_id;
        engine::core::ExchangeStatus new_status;
    };

    /// Handle a DISPATCH_FORM dispatch-channel message.
    ///
    /// Drives the state machine, persists the telegram row, and updates EDR if
    /// applicable.  Returns nullopt when the state machine rejects the telegram
    /// (no DB write is made in that case).
    [[nodiscard]] std::optional<DispatchOutcome> handle_dispatch_form(
        engine::core::DispatchFormType form, engine::core::TelegramDirection direction,
        const std::string& src_area, const std::string& dst_area, const std::string& train_number,
        const std::optional<std::string>& track_number, const std::vector<std::string>& km_markers,
        std::uint64_t timestamp_us);

    /// Handle a FREE_TEXT dispatch-channel message.
    ///
    /// Always persists to session.dispatch_telegrams (no state machine involved).
    void handle_free_text(const std::string& src_area, const std::string& dst_area,
                          const std::string& body, std::uint64_t timestamp_us);

private:
    DispatchExchangeManager& exchanges_;
    IDbWriter& db_writer_;
    EdrCoordinator& edr_;
    std::string session_id_;
};

}  // namespace server
