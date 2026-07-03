// server/include/server/dispatch_bus.hpp
// Consumes DeviceStateChange events from the ENGINE thread and broadcasts them
// as DOMAIN_EVENT (0x20) wire frames to all ACTIVE clients via TransportGateway.
// Simultaneously persists each emitted event to session.events via IDbWriter.
//
// DOMAIN_EVENT payload layout:
//   Offset  Size  Field
//    0       1    event_type
//    1       4    event_id  (uint32 LE, monotonically increasing, server-assigned)
//    5       8    timestamp_us (uint64 LE)
//   13       N    FlatBuffers body
//
// See docs/ARCHITECTURE.md — "DOMAIN_EVENT payload".

#pragma once

#include "engine/core/control_system.hpp"
#include "engine/core/engine_loop.hpp"
#include "server/db_writer.hpp"
#include "server/transport_gateway.hpp"

#include <atomic>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace server
{

class DispatchBus
{
public:
    /// Construct with DB persistence.
    /// db_writer and session_id are used to append every emitted domain event to
    /// session.events.  Pass NullDbWriter in unit tests.
    DispatchBus(TransportGateway& gateway, IDbWriter& db_writer, std::string session_id);

    // Returns the StateChangesCallback to pass to EngineLoop.
    // The returned functor is safe to store and invoke from the ENGINE thread.
    engine::core::EngineLoop::StateChangesCallback make_engine_callback();

    // Build the DOMAIN_EVENT prefix (event_type, event_id, timestamp_us).
    static std::vector<uint8_t> build_event_prefix(uint8_t event_type, uint32_t event_id,
                                                   uint64_t timestamp_us);

    // Serialize one DeviceStateChange into a complete DOMAIN_EVENT wire frame
    // (including the 16-byte transport header).
    // Returns nullopt for DeviceStateChange arms that have no direct event_type
    // mapping (e.g. SwitchLocked / SwitchUnlocked).
    std::optional<std::vector<uint8_t>> make_event_frame(
        const engine::core::DeviceStateChange& change, uint64_t timestamp_us);

private:
    void on_state_changes(const std::vector<engine::core::DeviceStateChange>& changes);

    /// Extract the primary object UID from a DeviceStateChange for DB logging.
    static std::optional<std::uint64_t> object_uid_from_change(
        const engine::core::DeviceStateChange& change);

    TransportGateway& gateway_;
    IDbWriter& db_writer_;
    std::string session_id_;
    std::atomic<uint32_t> next_event_id_{1};
    std::atomic<uint32_t> tx_seq_{0};
};

}  // namespace server
