#pragma once

#include "layout.hpp"

#include <optional>

namespace trackview
{

enum class OccupancyState
{
    Free,
    Occupied
};

enum class SwitchPositionState
{
    Straight,
    Divergent,
    Moving
};

enum class SignalIndicationState
{
    Stop,
    Proceed
};

struct TrackRuntimeState
{
    OccupancyState occupancy = OccupancyState::Free;
};

struct SwitchRuntimeState
{
    OccupancyState occupancy = OccupancyState::Free;
    SwitchPositionState position = SwitchPositionState::Straight;
};

struct SignalRuntimeState
{
    SignalIndicationState indication = SignalIndicationState::Stop;
};

// Read-only and minimal: rendering cannot mutate the simulation and does not
// gain access to routes, alarms, derailers or session internals.
class ITrackRuntimeState
{
public:
    virtual ~ITrackRuntimeState() = default;
    [[nodiscard]] virtual std::optional<TrackRuntimeState>
    track_state(InfrastructureId id) const noexcept = 0;
    [[nodiscard]] virtual std::optional<SwitchRuntimeState>
    switch_state(InfrastructureId id) const noexcept = 0;
    [[nodiscard]] virtual std::optional<SignalRuntimeState>
    signal_state(InfrastructureId id) const noexcept = 0;
};

}  // namespace trackview
