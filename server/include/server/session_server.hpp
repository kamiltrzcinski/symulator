// server/include/server/session_server.hpp
// Composition root for the simulator server process.
//
// SessionServer owns every subsystem and manages their lifecycle in the
// correct startup / shutdown order.  It is the only place where the wiring
// between subsystems lives — no subsystem knows about any other.
//
// Startup order:  load scenario  →  load fleet  →  resolve IControlSystem
//                 →  construct network layer  →  wire ENGINE callbacks
//                 →  start IO thread  →  start ENGINE thread
//
// Shutdown order: stop ENGINE  →  stop IO  →  release resources
//
// See docs/03-initial-architecture.md — Threading model.
// See docs/16-implementation-skeleton.md — Startup wiring sequence.

#pragma once

#include "server/dispatch_bus.hpp"
#include "server/ownership_guard.hpp"
#include "server/transport_gateway.hpp"

#include "engine/core/control_system.hpp"
#include "engine/core/engine_loop.hpp"
#include "engine/core/engine_snapshot.hpp"
#include "engine/core/engine_state.hpp"
#include "engine/core/fleet_registry.hpp"
#include "engine/core/priority_command_queue.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace server
{

struct SessionConfig
{
    std::filesystem::path scenario_dir;  ///< Scenario directory (meta.json, topology.json, …)
    std::filesystem::path data_dir;      ///< Fleet data root (vehicle_types/, vehicles/, trains/)
    uint16_t port = 9420;                ///< TCP listen port
};

// ── SessionServer ─────────────────────────────────────────────────────────────

class SessionServer
{
public:
    explicit SessionServer(SessionConfig config);
    ~SessionServer();

    // Non-copyable, non-movable — owns threads and file-descriptors.
    SessionServer(const SessionServer&) = delete;
    SessionServer& operator=(const SessionServer&) = delete;

    /// Parse argc/argv and construct a SessionServer.
    /// Exits the process (via std::exit) on --help or invalid arguments.
    static SessionServer from_args(int argc, char* argv[]);

    /// Start all subsystems and block until SIGINT or SIGTERM is received,
    /// then stop all subsystems.  This is the entire server lifecycle.
    void run();

private:
    void start();
    void stop();

    /// Build a COMMAND_NAK wire frame for broadcasting back to clients.
    /// Used as the ENGINE loop's nak_cb — called on the ENGINE thread.
    static std::vector<uint8_t> make_nak_frame(uint32_t seq_id,
                                               const engine::core::InterlockingViolation& v);

    SessionConfig config_;

    // ── Domain layer ─────────────────────────────────────────────────────────
    // No threads.  Loaded during start(); owned and mutated exclusively by the
    // ENGINE thread once the loop is running.

    engine::core::EngineState state_;
    engine::core::FleetRegistry fleet_;
    engine::core::PriorityCommandQueue<engine::core::EnvelopedCommand> cmd_queue_;
    engine::core::AtomicSnapshot snapshot_;

    // Null until start() resolves the IControlSystem from meta.json.
    std::unique_ptr<engine::core::IControlSystem> control_;

    // ── Network layer ─────────────────────────────────────────────────────────
    // Declaration order == construction order.
    // dispatch_bus_ holds a reference to gateway_, so it must be declared after
    // gateway_ (and therefore destroyed before it).

    OwnershipGuard ownership_;
    std::unique_ptr<TransportGateway> gateway_;
    std::unique_ptr<DispatchBus> dispatch_bus_;

    // ── ENGINE thread ─────────────────────────────────────────────────────────
    // Declared last: EngineLoop holds non-owning references to state_,
    // control_, cmd_queue_, snapshot_.  Its destructor joins the ENGINE thread
    // before any of those members are destroyed.

    std::unique_ptr<engine::core::EngineLoop> engine_loop_;
};

}  // namespace server
