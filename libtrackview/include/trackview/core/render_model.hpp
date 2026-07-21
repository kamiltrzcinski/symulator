#pragma once

#include "runtime_state.hpp"

#include <vector>

namespace trackview
{

struct RenderTrack
{
    InfrastructureId id;
    Path path;
    TrackRuntimeState state;
};

struct RenderSwitch
{
    InfrastructureId id;
    SwitchPorts ports;
    SwitchRuntimeState state;
};

struct RenderSignal
{
    InfrastructureId id;
    Point position;
    FacingDirection facing = FacingDirection::TowardsB;
    SignalRuntimeState state;
};

struct RenderModel
{
    Canvas canvas;
    std::vector<RenderTrack> tracks;
    std::vector<RenderSwitch> switches;
    std::vector<RenderSignal> signal_items;
    std::vector<LabelGeometry> labels;
};

}  // namespace trackview
