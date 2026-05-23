// server/include/server/transport_gateway.hpp
// TCP transport layer — accepts client connections, drives the per-client
// session state machine (HANDSHAKE → ACTIVE), and provides broadcast().
//
// Threading model:
//   - start() launches an asio io_context run loop on a dedicated thread
//     (IO_THREAD).  All asio handlers execute on that thread.
//   - broadcast() is safe to call from any thread; it posts work onto the
//     IO_THREAD via asio::post().
//   - stop() is safe to call from any thread; it stops the io_context and
//     joins the IO_THREAD.

#pragma once

// ASIO_STANDALONE is set project-wide via CMake; do not redefine here.
#include <asio.hpp>

#include "server/command_ingress.hpp"
#include "server/frame.hpp"
#include "server/ownership_guard.hpp"

#include "engine/core/command.hpp"
#include "engine/core/engine_snapshot.hpp"
#include "engine/core/priority_command_queue.hpp"
#include "engine/core/snapshot_service.hpp"

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace server
{

// ── ClientSession ─────────────────────────────────────────────────────────────

enum class SessionState
{
    kHandshake,  // Waiting for HANDSHAKE frame.
    kActive,     // Fully negotiated; commands and events flow.
    kClosing,    // Graceful close in progress.
};

struct ClientInfo
{
    std::string player_id;
    std::string session_id;
    std::string dispatch_area_id;  // populated at handshake; empty until set
    SessionState state = SessionState::kHandshake;
};

class TransportGateway;

class ClientSession : public std::enable_shared_from_this<ClientSession>
{
public:
    ClientSession(asio::ip::tcp::socket socket, TransportGateway& gateway);

    void start();
    void send(std::vector<uint8_t> frame);
    void close();

    const ClientInfo& info() const noexcept { return info_; }
    bool is_active() const noexcept { return info_.state == SessionState::kActive; }

private:
    void do_read();
    void on_read(std::error_code ec, std::size_t bytes);
    void process_buffer();
    void handle_frame(const DecodedFrame& frame);
    void handle_handshake(const DecodedFrame& frame);
    void handle_snapshot_request(const DecodedFrame& frame);
    void handle_command(const DecodedFrame& frame);
    void handle_heartbeat(const DecodedFrame& frame);
    void handle_bilateral(const DecodedFrame& frame);
    void send_frame(uint8_t msg_type, uint8_t flags, const std::vector<uint8_t>& payload);

    asio::ip::tcp::socket socket_;
    TransportGateway& gateway_;
    ClientInfo info_;

    // Per-client S→C sequence counter
    std::atomic<uint32_t> tx_seq_{0};

    // Read buffer
    static constexpr std::size_t kReadBufSize = 65536 + kHeaderSize;
    std::vector<uint8_t> read_buf_;
    std::size_t buf_fill_ = 0;

    // Write serialisation (strand-like: only IO_THREAD writes)
    std::mutex write_mutex_;
};

// ── TransportGateway ──────────────────────────────────────────────────────────

class TransportGateway
{
public:
    TransportGateway(engine::core::PriorityCommandQueue<engine::core::EnvelopedCommand>& cmd_queue,
                     OwnershipGuard& ownership, engine::core::AtomicSnapshot& snapshot);

    ~TransportGateway();

    // Non-copyable, non-movable.
    TransportGateway(const TransportGateway&) = delete;
    TransportGateway& operator=(const TransportGateway&) = delete;

    /// Start accepting connections on the given port.  Spawns the IO_THREAD.
    void start(uint16_t port);

    /// Stop the gateway; closes all sessions and joins the IO_THREAD.
    void stop();

    /// Broadcast a pre-encoded wire frame to all ACTIVE sessions.
    /// Thread-safe: may be called from any thread.
    void broadcast(std::vector<uint8_t> frame);

    /// Broadcast a frame only to ACTIVE sessions whose dispatch_area_id
    /// matches either src_area_id or dst_area_id.
    /// Thread-safe: may be called from any thread.
    void broadcast_to_pair(const std::string& src_area_id, const std::string& dst_area_id,
                           std::vector<uint8_t> frame);

    /// Register a handler for incoming msg_type 0x61 BILATERAL_MESSAGE frames.
    /// Called from IO_THREAD.  Must be set before start().
    using BilateralHandler =
        std::function<void(const std::vector<uint8_t>& payload, const std::string& sender_client_id,
                           const std::string& sender_area_id)>;
    void set_bilateral_handler(BilateralHandler handler);

    // ── Called by ClientSession ──────────────────────────────────────────────
    void register_session(std::shared_ptr<ClientSession> session);
    void unregister_session(ClientSession* session);

    const BilateralHandler& bilateral_handler() const noexcept { return bilateral_handler_; }
    engine::core::PriorityCommandQueue<engine::core::EnvelopedCommand>& cmd_queue() noexcept
    {
        return cmd_queue_;
    }
    OwnershipGuard& ownership() noexcept { return ownership_; }
    engine::core::AtomicSnapshot& snapshot() noexcept { return snapshot_; }

private:
    void do_accept();

    engine::core::PriorityCommandQueue<engine::core::EnvelopedCommand>& cmd_queue_;
    OwnershipGuard& ownership_;
    engine::core::AtomicSnapshot& snapshot_;

    asio::io_context io_ctx_;
    asio::ip::tcp::acceptor acceptor_{io_ctx_};

    std::mutex sessions_mutex_;
    std::unordered_map<ClientSession*, std::shared_ptr<ClientSession>> sessions_;

    std::thread io_thread_;
    std::atomic<bool> running_{false};

    BilateralHandler bilateral_handler_;
};

}  // namespace server
