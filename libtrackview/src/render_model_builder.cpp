#include <trackview/core/render_model_builder.hpp>

#include <stdexcept>
#include <type_traits>
#include <unordered_map>

namespace trackview
{

RenderModel RenderModelBuilder::build(const ITrackRuntimeState& state,
                                      const TrackLayout& layout) const
{
    RenderModel model;
    model.canvas = layout.canvas;
    model.labels = layout.labels;

    std::unordered_map<InfrastructureId, const Path*> track_paths;
    for (const auto& element : layout.elements)
        if (const auto* track = std::get_if<TrackSectionGeometry>(&element))
            track_paths.emplace(track->topology_id, &track->path);

    for (const auto& element : layout.elements)
    {
        std::visit(
            [&](const auto& item) {
                using T = std::decay_t<decltype(item)>;
                if constexpr (std::is_same_v<T, TrackSectionGeometry>)
                {
                    const auto runtime = state.track_state(item.topology_id);
                    if (!runtime)
                        throw std::runtime_error("render_model_builder: missing track state " +
                                                 std::to_string(item.topology_id.value));
                    model.tracks.push_back({item.topology_id, item.path, *runtime});
                }
                else if constexpr (std::is_same_v<T, SwitchGeometry>)
                {
                    const auto runtime = state.switch_state(item.topology_id);
                    if (!runtime)
                        throw std::runtime_error("render_model_builder: missing switch state " +
                                                 std::to_string(item.topology_id.value));
                    model.switches.push_back({item.topology_id, item.ports, *runtime});
                }
                else if constexpr (std::is_same_v<T, SignalGeometry>)
                {
                    const auto runtime = state.signal_state(item.object_id);
                    if (!runtime)
                        throw std::runtime_error("render_model_builder: missing signal state " +
                                                 std::to_string(item.object_id.value));
                    const auto path = track_paths.find(item.attachment.track_id);
                    if (path == track_paths.end())
                        throw std::runtime_error(
                            "render_model_builder: signal track geometry is missing");
                    model.signal_items.push_back(
                        {item.object_id,
                         attachment_resolver_.resolve(*path->second, item.attachment),
                         item.facing, *runtime});
                }
            },
            element);
    }
    return model;
}

}  // namespace trackview
