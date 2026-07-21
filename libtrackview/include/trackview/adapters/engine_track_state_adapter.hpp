#pragma once

#include <trackview/core/infrastructure_catalog.hpp>
#include <trackview/core/runtime_state.hpp>

#include <engine/core/state_view.hpp>

namespace trackview
{

class EngineInfrastructureCatalogAdapter final : public IInfrastructureCatalog
{
public:
    explicit EngineInfrastructureCatalogAdapter(const engine::core::IStateView& source)
        : source_(source)
    {
    }

    [[nodiscard]] bool contains_track(InfrastructureId id) const noexcept override;
    [[nodiscard]] bool contains_switch(InfrastructureId id) const noexcept override;
    [[nodiscard]] bool contains_signal(InfrastructureId id) const noexcept override;
    [[nodiscard]] bool signal_governs_track(InfrastructureId signal_id,
                                            InfrastructureId track_id) const noexcept override;

private:
    const engine::core::IStateView& source_;
};

class EngineTrackRuntimeAdapter final : public ITrackRuntimeState
{
public:
    explicit EngineTrackRuntimeAdapter(const engine::core::IStateView& source)
        : source_(source)
    {
    }

    [[nodiscard]] std::optional<TrackRuntimeState>
    track_state(InfrastructureId id) const noexcept override;
    [[nodiscard]] std::optional<SwitchRuntimeState>
    switch_state(InfrastructureId id) const noexcept override;
    [[nodiscard]] std::optional<SignalRuntimeState>
    signal_state(InfrastructureId id) const noexcept override;

private:
    const engine::core::IStateView& source_;
};

}  // namespace trackview
