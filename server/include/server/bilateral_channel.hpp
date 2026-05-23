#pragma once

// server/include/server/bilateral_channel.hpp
//
// Handles msg_type 0x61 BILATERAL_MESSAGE frames.
//
// Responsibilities:
//   1. Parse the FlatBuffers payload.
//   2. Forward the telegram to DispatchExchangeManager.
//   3. Persist accepted telegrams via IDbWriter.
//   4. Broadcast the result frame to the (src_area, dst_area) pair.
//
// Must be driven from the IO_THREAD (same thread as TransportGateway callbacks).

#include "server/db_writer.hpp"
#include "server/dispatch_exchange_manager.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace server
{

class TransportGateway;

class BilateralChannel
{
public:
    BilateralChannel(DispatchExchangeManager& exchanges, IDbWriter& db_writer,
                     TransportGateway& gateway, std::string session_id);

    /// Called from ClientSession::handle_frame for msg_type 0x61.
    /// @param payload  Raw FlatBuffers bytes (without the 16-byte wire header).
    /// @param sender_client_id  client_id from ClientInfo.player_id.
    /// @param sender_area_id    dispatch_area_id from ClientInfo.dispatch_area_id.
    void on_inbound(const std::vector<uint8_t>& payload, const std::string& sender_client_id,
                    const std::string& sender_area_id);

private:
    DispatchExchangeManager& exchanges_;
    IDbWriter& db_writer_;
    TransportGateway& gateway_;
    std::string session_id_;
};

}  // namespace server
