// server/src/session_server.cpp

#include "server/session_server.hpp"

#include "engine/core/control_system_registry.hpp"
#include "engine/core/topology_loader.hpp"
#include "server/frame.hpp"

// FlatBuffers generated header for CommandNak / NakReason
#include <flatbuffers/flatbuffers.h>
#include "commands_generated.h"

#include <signal.h>

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace server
{

// ── from_args ─────────────────────────────────────────────────────────────────

static void print_usage(std::ostream& out)
{
    out << "Usage: simserver --scenario <dir> [--data <dir>] [--port <N>]\n"
           "  --scenario / -s  path to scenario directory   (required)\n"
           "  --data     / -d  path to fleet data root       (default: ./data)\n"
           "  --port     / -p  TCP listen port               (default: 9420)\n";
}

SessionServer SessionServer::from_args(int argc, char* argv[])
{
    SessionConfig cfg;
    cfg.data_dir = std::filesystem::current_path() / "data";

    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg(argv[i]);
        if ((arg == "--scenario" || arg == "-s") && i + 1 < argc)
            cfg.scenario_dir = argv[++i];
        else if ((arg == "--data" || arg == "-d") && i + 1 < argc)
            cfg.data_dir = argv[++i];
        else if ((arg == "--port" || arg == "-p") && i + 1 < argc)
            cfg.port = static_cast<uint16_t>(std::stoi(argv[++i]));
        else if (arg == "--help" || arg == "-h")
        {
            print_usage(std::cout);
            std::exit(0);
        }
    }

    if (cfg.scenario_dir.empty())
    {
        std::cerr << "error: --scenario is required.\n";
        print_usage(std::cerr);
        std::exit(1);
    }

    return SessionServer(std::move(cfg));
}

// ── run ───────────────────────────────────────────────────────────────────────

void SessionServer::run()
{
    start();

    // Cross-platform signal handling using asio::signal_set
    asio::io_context sig_io;
    asio::signal_set signals(sig_io, SIGINT, SIGTERM);
    signals.async_wait([&](const std::error_code&, int sig)
    {
        std::cout << "\n[server] Signal " << sig << " received, shutting down…\n";
        sig_io.stop();
    });
    sig_io.run();

    stop();
}


// ── Constructor / destructor ──────────────────────────────────────────────────

SessionServer::SessionServer(SessionConfig config) : config_(std::move(config)) {}

SessionServer::~SessionServer()
{
    stop();
}

// ── start ─────────────────────────────────────────────────────────────────────

void SessionServer::start()
{
    // 1. Load topology → EngineState.
    std::cout << "[server] Loading scenario: " << config_.scenario_dir << "\n";
    const auto meta = engine::core::load_scenario(state_, config_.scenario_dir);
    state_.set_session_id(meta.station_sid);
    std::cout << "[server] Station: " << meta.station_sid
              << "  control_system: " << meta.control_system_id << "\n";

    // 2. Load fleet data (vehicle types, instances, consists).
    fleet_.load(config_.data_dir);
    std::cout << "[server] Fleet loaded from: " << config_.data_dir << "\n";

    // 3. Resolve IControlSystem from self-registered SRK libraries.
    auto& reg = engine::core::ControlSystemRegistry::instance();
    if (!reg.has(engine::core::ControlSystemID{meta.control_system_id}))
        throw std::runtime_error("[server] Unknown control_system: " + meta.control_system_id);
    control_ = reg.create(engine::core::ControlSystemID{meta.control_system_id});
    std::cout << "[server] IControlSystem: " << meta.control_system_id << "\n";

    // 4. Construct network layer.
    gateway_ = std::make_unique<TransportGateway>(cmd_queue_, ownership_, snapshot_);
    dispatch_bus_ = std::make_unique<DispatchBus>(*gateway_);

    // 5. Wire ENGINE callbacks.
    //    nak_cb   — called on ENGINE thread when a command fails interlocking.
    //    changes_cb — called on ENGINE thread once per tick with all state changes.
    //    pip_cb   — called on ENGINE thread with PipEvents from TrainFleet.
    auto nak_cb = [this](const engine::core::EnvelopedCommand& cmd,
                         const engine::core::InterlockingViolation& violation)
    {
        // Broadcast so the originating client can filter by seq_id.
        gateway_->broadcast(make_nak_frame(cmd.meta.seq_id, violation));
    };

    auto pip_cb = [](const std::vector<engine::core::PipEvent>& events)
    {
        for (const auto& ev : events)
        {
            std::cerr << "[PIP] section=" << ev.section_gid.value
                      << " station=" << ev.station_sid.value << " occ="
                      << (ev.occupancy == engine::core::TrackOccupancy::OCCUPIED ? "OCC" : "FREE");
            if (ev.slot)
                std::cerr << " train=" << ev.slot->number;
            if (ev.lcs_boundary_crossing)
                std::cerr << " [boundary]";
            std::cerr << "\n";
        }
    };

    engine_loop_ = std::make_unique<engine::core::EngineLoop>(
        state_, *control_, cmd_queue_, snapshot_, std::move(nak_cb),
        dispatch_bus_->make_engine_callback(), std::move(pip_cb));

    // 6. Start threads — IO first so the port is open before ENGINE begins ticking.
    gateway_->start(config_.port);
    engine_loop_->start();

    std::cout << "[server] Listening on port " << config_.port << "\n";
}

// ── stop ──────────────────────────────────────────────────────────────────────

void SessionServer::stop()
{
    // Reverse startup order: ENGINE → IO → resources.
    if (engine_loop_)
    {
        engine_loop_->stop();
        engine_loop_.reset();
    }

    if (gateway_)
    {
        gateway_->stop();
        gateway_.reset();
    }

    dispatch_bus_.reset();
    control_.reset();

    // Unblock any WORK_POOL thread still trying to push a command.
    cmd_queue_.close();

    std::cout << "[server] Stopped.\n";
}

// ── Helpers ───────────────────────────────────────────────────────────────────

std::vector<uint8_t> SessionServer::make_nak_frame(uint32_t seq_id,
                                                   const engine::core::InterlockingViolation& v)
{
    flatbuffers::FlatBufferBuilder fbb(256);
    auto text_off = fbb.CreateString(v.reason_text);
    const auto reason = static_cast<proto::NakReason>(v.reason_code);
    auto nak_off = proto::CreateCommandNak(fbb, seq_id, reason, text_off);
    fbb.Finish(nak_off);

    return encode_frame(msg_type::kCommandNak, 0, 0, fbb.GetBufferPointer(), fbb.GetSize());
}

}  // namespace server
