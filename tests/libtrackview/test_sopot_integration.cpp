#include <engine/core/engine_state.hpp>
#include <engine/core/topology_loader.hpp>

#include <trackview/adapters/engine_track_state_adapter.hpp>
#include <trackview/core/attachment_resolver.hpp>
#include <trackview/core/layout_reader.hpp>
#include <trackview/core/layout_validator.hpp>
#include <trackview/core/render_model_builder.hpp>
#include <trackview/qt/scene_renderer.hpp>

#include <gtest/gtest.h>

#include <filesystem>

namespace
{

TEST(SopotTrackViewIntegration, LoadsValidatesAndRendersRealScenario)
{
    engine::core::EngineState engine_state;
    const auto meta = engine::core::load_scenario(engine_state, SOPOT_SCENARIO_DIR);
    const auto layout = trackview::JsonLayoutReader{}.read(
        std::filesystem::path(SOPOT_SCENARIO_DIR) / "layouts" / "ebiscreen.json");
    ASSERT_EQ(layout.station_sid, meta.station_sid);

    const trackview::EngineInfrastructureCatalogAdapter catalog(engine_state);
    const auto diagnostics = trackview::LayoutValidator{}.validate(catalog, layout);
    ASSERT_TRUE(diagnostics.empty());

    const trackview::EngineTrackRuntimeAdapter runtime(engine_state);
    const trackview::PathAttachmentResolver attachment_resolver;
    const auto model =
        trackview::RenderModelBuilder(attachment_resolver).build(runtime, layout);
    EXPECT_EQ(model.tracks.size(), 16U);
    EXPECT_EQ(model.switches.size(), 8U);
    EXPECT_EQ(model.signal_items.size(), 16U);

    const trackview::EbiScreenTheme theme;
    const trackview::QtSceneRenderer scene_renderer(theme);
    const trackview::QtImageRenderer image_renderer(scene_renderer);
    const auto image = image_renderer.render(model, 4.0);
    EXPECT_FALSE(image.isNull());
    EXPECT_GT(image.width(), 300);
}

}  // namespace
