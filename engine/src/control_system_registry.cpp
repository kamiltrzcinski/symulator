#include "engine/core/control_system_registry.hpp"

#include <stdexcept>

namespace engine::core
{

ControlSystemRegistry& ControlSystemRegistry::instance()
{
    static ControlSystemRegistry reg;
    return reg;
}

void ControlSystemRegistry::register_system(ControlSystemID id, FactoryFn factory)
{
    if (factories_.count(id.value))
        throw std::logic_error("ControlSystemRegistry: ID '" + id.value + "' already registered");
    factories_.emplace(id.value, std::move(factory));
}

std::unique_ptr<IControlSystem> ControlSystemRegistry::create(const ControlSystemID& id) const
{
    auto it = factories_.find(id.value);
    if (it == factories_.end())
        throw std::out_of_range("ControlSystemRegistry: unknown system ID '" + id.value + "'");
    return it->second();
}

bool ControlSystemRegistry::has(const ControlSystemID& id) const
{
    return factories_.count(id.value) != 0;
}

bool ControlSystemRegistry::register_static(ControlSystemID id, FactoryFn factory)
{
    instance().register_system(std::move(id), std::move(factory));
    return true;
}

}  // namespace engine::core
