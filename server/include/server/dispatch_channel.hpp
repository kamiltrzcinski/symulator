#pragma once

// server/include/server/dispatch_channel.hpp
//
// Handles msg_type 0x61 DISPATCH_CHANNEL_MESSAGE frames.
//
// Responsibilities:
//   1. Parse the FlatBuffers payload.
//   2. Verify the sender's claimed src_area_id matches the authenticated area.
//   3. Delegate business logic to DispatchCoordinator.
//   4. Broadcast the result frame to the (src_area, dst_area) pair.
//
// Must be driven from the IO_THREAD (same thread as TransportGateway callbacks).

#include "server/dispatch_coordinator.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace server
{

class TransportGateway;

class DispatchChannel
{
public:
    DispatchChannel(DispatchCoordinator& coordinator, TransportGateway& gateway);

    /// Called from ClientSession::handle_frame for msg_type 0x61.
    /// @param payload           Raw FlatBuffers bytes (without the 16-byte wire header).
    /// @param sender_client_id  client_id from ClientInfo.player_id.
    /// @param sender_area_id    dispatch_area_id from ClientInfo.dispatch_area_id.
    void on_inbound(const std::vector<uint8_t>& payload, const std::string& sender_client_id,
                    const std::string& sender_area_id);

private:
    DispatchCoordinator& coordinator_;
    TransportGateway& gateway_;
};

}  // namespace server
