#pragma once

#include <trackview/core/attachment_resolver.hpp>
#include <trackview/core/infrastructure_catalog.hpp>
#include <trackview/core/runtime_state.hpp>

#include <unordered_map>
#include <unordered_set>

namespace trackview::test
{

class FakeCatalog final : public IInfrastructureCatalog
{
public:
    std::unordered_set<InfrastructureId> tracks;
    std::unordered_set<InfrastructureId> switches;
    std::unordered_set<InfrastructureId> signal_items;
    std::unordered_map<InfrastructureId, InfrastructureId> governed_tracks;

    bool contains_track(InfrastructureId id) const noexcept override
    {
        return tracks.contains(id);
    }
    bool contains_switch(InfrastructureId id) const noexcept override
    {
        return switches.contains(id);
    }
    bool contains_signal(InfrastructureId id) const noexcept override
    {
        return signal_items.contains(id);
    }
    bool signal_governs_track(InfrastructureId signal_id,
                              InfrastructureId track_id) const noexcept override
    {
        const auto found = governed_tracks.find(signal_id);
        return found != governed_tracks.end() && found->second == track_id;
    }
};

class FakeRuntimeState final : public ITrackRuntimeState
{
public:
    std::unordered_map<InfrastructureId, TrackRuntimeState> tracks;
    std::unordered_map<InfrastructureId, SwitchRuntimeState> switches;
    std::unordered_map<InfrastructureId, SignalRuntimeState> signal_items;

    std::optional<TrackRuntimeState>
    track_state(InfrastructureId id) const noexcept override
    {
        const auto found = tracks.find(id);
        return found == tracks.end() ? std::nullopt
                                     : std::optional<TrackRuntimeState>(found->second);
    }
    std::optional<SwitchRuntimeState>
    switch_state(InfrastructureId id) const noexcept override
    {
        const auto found = switches.find(id);
        return found == switches.end() ? std::nullopt
                                       : std::optional<SwitchRuntimeState>(found->second);
    }
    std::optional<SignalRuntimeState>
    signal_state(InfrastructureId id) const noexcept override
    {
        const auto found = signal_items.find(id);
        return found == signal_items.end()
                   ? std::nullopt
                   : std::optional<SignalRuntimeState>(found->second);
    }
};

class FixedAttachmentResolver final : public IAttachmentResolver
{
public:
    explicit FixedAttachmentResolver(Point result) : result_(result) {}

    Point resolve(const Path&, const TrackAttachment&) const override { return result_; }

private:
    Point result_;
};

}  // namespace trackview::test
