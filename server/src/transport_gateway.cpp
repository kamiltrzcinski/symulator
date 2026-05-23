// server/src/transport_gateway.cpp

// ASIO_STANDALONE is set project-wide via CMake; do not redefine here.
#include "server/transport_gateway.hpp"

#include "commands_generated.h"
#include "session_generated.h"

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/verifier.h>

#include <chrono>
#include <iostream>
#include <sstream>

namespace server
{

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::vector<uint8_t> build_flatbuffers_payload(const flatbuffers::FlatBufferBuilder& fbb)
{
    return {fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize()};
}

static uint64_t now_us()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count());
}

// ── ClientSession ─────────────────────────────────────────────────────────────

ClientSession::ClientSession(asio::ip::tcp::socket socket, TransportGateway& gateway)
    : socket_(std::move(socket)), gateway_(gateway)
{
    read_buf_.resize(kReadBufSize);
}

void ClientSession::start()
{
    do_read();
}

void ClientSession::do_read()
{
    auto self = shared_from_this();
    socket_.async_read_some(
        asio::buffer(read_buf_.data() + buf_fill_, read_buf_.size() - buf_fill_),
        [self](std::error_code ec, std::size_t bytes) { self->on_read(ec, bytes); });
}

void ClientSession::on_read(std::error_code ec, std::size_t bytes)
{
    if (ec)
    {
        gateway_.unregister_session(this);
        return;
    }
    buf_fill_ += bytes;
    process_buffer();
    if (info_.state != SessionState::kClosing)
        do_read();
}

void ClientSession::process_buffer()
{
    while (buf_fill_ > 0)
    {
        auto result = decode_frame(read_buf_.data(), buf_fill_);
        if (result.status == FrameDecodeStatus::kNeedMoreData)
            break;

        if (result.status != FrameDecodeStatus::kOk)
        {
            // Protocol error — drop this connection
            close();
            return;
        }

        handle_frame(result.frame);

        // Consume the parsed frame from the buffer
        const std::size_t consumed = result.bytes_consumed;
        std::memmove(read_buf_.data(), read_buf_.data() + consumed, buf_fill_ - consumed);
        buf_fill_ -= consumed;
    }
}

void ClientSession::handle_frame(const DecodedFrame& frame)
{
    switch (frame.msg_type)
    {
        case msg_type::kHandshake:
            handle_handshake(frame);
            break;
        case msg_type::kSnapshotRequest:
            if (is_active())
                handle_snapshot_request(frame);
            break;
        case msg_type::kCommand:
            if (is_active())
                handle_command(frame);
            break;
        case msg_type::kHeartbeat:
            handle_heartbeat(frame);
            break;
        default:
            break;  // ignore unknown / unimplemented types
    }
}

void ClientSession::handle_handshake(const DecodedFrame& frame)
{
    if (frame.payload.empty())
    {
        close();
        return;
    }

    flatbuffers::Verifier verifier(frame.payload.data(), frame.payload.size());
    if (!verifier.VerifyBuffer<proto::Handshake>())
    {
        close();
        return;
    }

    const auto* hs = flatbuffers::GetRoot<proto::Handshake>(frame.payload.data());
    if (!hs->player_id() || !hs->auth_token())
    {
        close();
        return;
    }

    info_.player_id = hs->player_id()->str();
    info_.session_id = "SID-" + info_.player_id + "-" + std::to_string(now_us());
    info_.state = SessionState::kActive;

    // Build HANDSHAKE_ACK
    flatbuffers::FlatBufferBuilder fbb(256);
    auto sid_off = fbb.CreateString(info_.session_id);
    auto ack_off = proto::CreateHandshakeAck(fbb, sid_off, /*server_tick_hz=*/20,
                                             /*assigned_dispatch_areas=*/0,
                                             /*server_time_us=*/now_us());
    fbb.Finish(ack_off);

    send_frame(msg_type::kHandshakeAck, 0, build_flatbuffers_payload(fbb));
}

void ClientSession::handle_snapshot_request(const DecodedFrame& frame)
{
    auto snap = gateway_.snapshot().load();
    if (!snap)
        return;  // no snapshot yet — client should retry

    const auto serialised = engine::core::SnapshotService::serialize(*snap);
    const auto chunks = engine::core::SnapshotService::chunk(serialised);

    for (std::size_t i = 0; i < chunks.size(); ++i)
    {
        const bool is_last = (i + 1 == chunks.size());
        const uint8_t flags = is_last ? kFlagIsLastChunk : 0u;
        send_frame(msg_type::kSnapshotChunk, flags, chunks[i]);
    }
}

void ClientSession::handle_command(const DecodedFrame& frame)
{
    if (frame.payload.empty())
        return;

    auto cmd_opt = CommandIngress::parse_payload(frame.payload.data(),
                                                 static_cast<uint32_t>(frame.payload.size()));
    if (!cmd_opt)
    {
        // Unknown command type — send NAK
        flatbuffers::FlatBufferBuilder fbb(64);
        auto text_off = fbb.CreateString("Unsupported command type");
        auto nak_off =
            proto::CreateCommandNak(fbb, frame.seq_id, proto::NakReason_UNSUPPORTED, text_off);
        fbb.Finish(nak_off);
        send_frame(msg_type::kCommandNak, 0, build_flatbuffers_payload(fbb));
        return;
    }

    engine::core::CommandMeta meta;
    meta.seq_id = frame.seq_id;
    meta.priority = engine::core::CommandPriority::NORMAL;
    meta.player_id = engine::core::PlayerID{info_.player_id};
    meta.timestamp_us = now_us();

    engine::core::EnvelopedCommand env{meta, std::move(*cmd_opt)};
    try
    {
        gateway_.cmd_queue().push(std::move(env), engine::core::CommandPriority::NORMAL);
    }
    catch (const std::runtime_error&)
    {
        return;  // queue closed — server is shutting down
    }

    // Optimistic ACK
    flatbuffers::FlatBufferBuilder fbb(32);
    auto ack_off = proto::CreateCommandAck(fbb, frame.seq_id);
    fbb.Finish(ack_off);
    send_frame(msg_type::kCommandAck, 0, build_flatbuffers_payload(fbb));
}

void ClientSession::handle_heartbeat(const DecodedFrame& frame)
{
    flatbuffers::FlatBufferBuilder fbb(32);
    auto ack_off = proto::CreateHeartbeatAck(fbb, frame.seq_id, now_us());
    fbb.Finish(ack_off);
    send_frame(msg_type::kHeartbeatAck, 0, build_flatbuffers_payload(fbb));
}

void ClientSession::send_frame(uint8_t mt, uint8_t flags, const std::vector<uint8_t>& payload)
{
    const uint32_t seq = tx_seq_.fetch_add(1, std::memory_order_relaxed);
    send(encode_frame(mt, flags, seq, payload));
}

void ClientSession::send(std::vector<uint8_t> frame)
{
    // Post a write onto the IO thread to avoid concurrent socket writes.
    auto self = shared_from_this();
    auto data = std::make_shared<std::vector<uint8_t>>(std::move(frame));
    asio::post(socket_.get_executor(),
               [self, data]()
               {
                   std::scoped_lock lock{self->write_mutex_};
                   asio::write(self->socket_, asio::buffer(*data));
               });
}

void ClientSession::close()
{
    info_.state = SessionState::kClosing;
    std::error_code ec;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
    gateway_.unregister_session(this);
}

// ── TransportGateway ──────────────────────────────────────────────────────────

TransportGateway::TransportGateway(
    engine::core::PriorityCommandQueue<engine::core::EnvelopedCommand>& cmd_queue,
    OwnershipGuard& ownership, engine::core::AtomicSnapshot& snapshot)
    : cmd_queue_(cmd_queue), ownership_(ownership), snapshot_(snapshot)
{
}

TransportGateway::~TransportGateway()
{
    stop();
}

void TransportGateway::start(uint16_t port)
{
    if (running_.exchange(true))
        return;

    const asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), port);
    acceptor_.open(ep.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(ep);
    acceptor_.listen();

    do_accept();

    io_thread_ = std::thread([this]() { io_ctx_.run(); });
}

void TransportGateway::stop()
{
    if (!running_.exchange(false))
        return;

    acceptor_.close();
    io_ctx_.stop();

    if (io_thread_.joinable())
        io_thread_.join();

    std::scoped_lock lock{sessions_mutex_};
    for (auto& [ptr, session] : sessions_)
        session->close();
    sessions_.clear();
}

void TransportGateway::do_accept()
{
    acceptor_.async_accept(
        [this](std::error_code ec, asio::ip::tcp::socket socket)
        {
            if (!ec)
            {
                auto session = std::make_shared<ClientSession>(std::move(socket), *this);
                register_session(session);
                session->start();
            }
            if (acceptor_.is_open())
                do_accept();
        });
}

void TransportGateway::broadcast(std::vector<uint8_t> frame)
{
    // Copy the frame and post to the IO thread so socket writes are serialised.
    auto data = std::make_shared<std::vector<uint8_t>>(std::move(frame));

    asio::post(io_ctx_,
               [this, data]()
               {
                   std::scoped_lock lock{sessions_mutex_};
                   for (auto& [ptr, session] : sessions_)
                   {
                       if (session->is_active())
                           session->send(*data);
                   }
               });
}

void TransportGateway::register_session(std::shared_ptr<ClientSession> session)
{
    std::scoped_lock lock{sessions_mutex_};
    sessions_[session.get()] = std::move(session);
}

void TransportGateway::unregister_session(ClientSession* session)
{
    std::scoped_lock lock{sessions_mutex_};
    sessions_.erase(session);
}

}  // namespace server
