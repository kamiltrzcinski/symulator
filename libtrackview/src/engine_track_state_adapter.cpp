#include <trackview/adapters/engine_track_state_adapter.hpp>

namespace trackview
{
namespace
{

engine::core::UID to_engine_uid(InfrastructureId id)
{
    return engine::core::UID{id.value};
}

OccupancyState map_occupancy(engine::core::TrackOccupancy state)
{
    return state == engine::core::TrackOccupancy::OCCUPIED ? OccupancyState::Occupied
                                                            : OccupancyState::Free;
}

SwitchPositionState map_position(engine::core::SwitchPosition state)
{
    switch (state)
    {
    case engine::core::SwitchPosition::STRAIGHT:
        return SwitchPositionState::Straight;
    case engine::core::SwitchPosition::DIVERGENT:
        return SwitchPositionState::Divergent;
    case engine::core::SwitchPosition::MOVING:
        return SwitchPositionState::Moving;
    }
    return SwitchPositionState::Moving;
}

SignalIndicationState map_indication(engine::core::SignalAspect aspect)
{
    return aspect == engine::core::SignalAspect::S1_STOP ||
                   aspect == engine::core::SignalAspect::MS1_STOP
               ? SignalIndicationState::Stop
               : SignalIndicationState::Proceed;
}

}  // namespace

bool EngineInfrastructureCatalogAdapter::contains_track(InfrastructureId id) const noexcept
{
    return source_.find_track_section(to_engine_uid(id)) != nullptr;
}

bool EngineInfrastructureCatalogAdapter::contains_switch(InfrastructureId id) const noexcept
{
    return source_.find_switch(to_engine_uid(id)) != nullptr;
}

bool EngineInfrastructureCatalogAdapter::contains_signal(InfrastructureId id) const noexcept
{
    return source_.find_signal(to_engine_uid(id)) != nullptr;
}

bool EngineInfrastructureCatalogAdapter::signal_governs_track(
    InfrastructureId signal_id, InfrastructureId track_id) const noexcept
{
    const auto* signal_state = source_.find_signal(to_engine_uid(signal_id));
    return signal_state && signal_state->governs_section_uid == to_engine_uid(track_id);
}

std::optional<TrackRuntimeState>
EngineTrackRuntimeAdapter::track_state(InfrastructureId id) const noexcept
{
    const auto* state = source_.find_track_section(to_engine_uid(id));
    if (!state)
        return std::nullopt;
    return TrackRuntimeState{map_occupancy(state->occupancy)};
}

std::optional<SwitchRuntimeState>
EngineTrackRuntimeAdapter::switch_state(InfrastructureId id) const noexcept
{
    const auto* state = source_.find_switch(to_engine_uid(id));
    if (!state)
        return std::nullopt;
    return SwitchRuntimeState{map_occupancy(state->occupancy), map_position(state->position)};
}

std::optional<SignalRuntimeState>
EngineTrackRuntimeAdapter::signal_state(InfrastructureId id) const noexcept
{
    const auto* state = source_.find_signal(to_engine_uid(id));
    if (!state)
        return std::nullopt;
    return SignalRuntimeState{map_indication(state->current_aspect)};
}

}  // namespace trackview
