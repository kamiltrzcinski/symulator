#include "engine/core/control_system_registry.hpp"

#include <stdexcept>

namespace engine::core
{

ControlSystemRegistry& ControlSystemRegistry::instance()
{
    static ControlSystemRegistry reg;
    return reg;
}

void ControlSystemRegistry::register_system(std::string id, FactoryFn factory)
{
    if (factories_.count(id))
        throw std::logic_error("ControlSystemRegistry: ID '" + id + "' already registered");
    factories_.emplace(std::move(id), std::move(factory));
}

std::unique_ptr<IControlSystem> ControlSystemRegistry::create(const std::string& id) const
{
    auto it = factories_.find(id);
    if (it == factories_.end())
        throw std::out_of_range("ControlSystemRegistry: unknown system ID '" + id + "'");
    return it->second();
}

bool ControlSystemRegistry::has(const std::string& id) const
{
    return factories_.count(id) != 0;
}

bool ControlSystemRegistry::register_static(std::string id, FactoryFn factory)
{
    instance().register_system(std::move(id), std::move(factory));
    return true;
}

}  // namespace engine::core
