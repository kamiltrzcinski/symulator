#pragma once

#include "layout.hpp"

namespace trackview
{

// Deliberately contains existence queries only. Validation does not depend on
// runtime state or the engine's much larger state-view interface.
class IInfrastructureCatalog
{
public:
    virtual ~IInfrastructureCatalog() = default;
    [[nodiscard]] virtual bool contains_track(InfrastructureId id) const noexcept = 0;
    [[nodiscard]] virtual bool contains_switch(InfrastructureId id) const noexcept = 0;
    [[nodiscard]] virtual bool contains_signal(InfrastructureId id) const noexcept = 0;
    [[nodiscard]] virtual bool signal_governs_track(InfrastructureId signal_id,
                                                    InfrastructureId track_id) const noexcept = 0;
};

}  // namespace trackview
