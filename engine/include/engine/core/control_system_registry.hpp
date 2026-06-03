#pragma once

#include "control_system.hpp"

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// ── ControlSystemRegistry ────────────────────────────────────────────────────
// Singleton factory that maps control system ID strings to IControlSystem
// factory functions.
//
// Each SRK library registers itself at static-initialisation time via
// ControlSystemRegistry::register_static().  The engine then calls create()
// with the ID found in meta.json to obtain the concrete implementation.
//
// Design:
//   - Not thread-safe for registration (register_static runs before main).
//   - Thread-safe for create() (read-only after registration).

namespace engine::core
{

class ControlSystemRegistry
{
public:
    using FactoryFn = std::function<std::unique_ptr<IControlSystem>()>;

    // Returns the process-wide singleton.
    static ControlSystemRegistry& instance();

    // Register a factory for the given system ID.
    // Throws std::logic_error if the ID is already registered.
    void register_system(std::string id, FactoryFn factory);

    // Create a new instance of the named control system.
    // Throws std::out_of_range if the ID is not registered.
    std::unique_ptr<IControlSystem> create(const std::string& id) const;

    // Returns true if the ID is registered.
    bool has(const std::string& id) const;

    // Convenience helper for use in static initialisers:
    //   static bool _reg = ControlSystemRegistry::register_static(
    //       "ebilock_x4", []{ return std::make_unique<EbiLockSystem>(); });
    //
    // Returns true so that the result can be assigned to a (discarded) static bool.
    static bool register_static(std::string id, FactoryFn factory);

private:
    ControlSystemRegistry() = default;

    std::unordered_map<std::string, FactoryFn> factories_;
};

}  // namespace engine::core
