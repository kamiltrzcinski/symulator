#include <trackview/qt/scene_renderer.hpp>

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>

#include <gtest/gtest.h>

namespace
{

class MagentaTheme final : public trackview::ITrackTheme
{
public:
    QBrush background() const override { return QBrush(Qt::black); }
    QPen track_pen(trackview::OccupancyState) const override
    {
        return QPen(Qt::magenta, 2.0);
    }
    QPen switch_pen(trackview::OccupancyState, bool) const override
    {
        return QPen(Qt::cyan, 1.0);
    }
    QBrush signal_brush(trackview::SignalIndicationState) const override
    {
        return QBrush(Qt::yellow);
    }
    QColor label_colour() const override { return QColor(Qt::white); }
};

class StubSceneRenderer final : public trackview::ISceneRenderer
{
public:
    void render(const trackview::RenderModel& model,
                QGraphicsScene& scene) const override
    {
        ++calls;
        scene.setSceneRect(0, 0, model.canvas.width, model.canvas.height);
        scene.setBackgroundBrush(Qt::black);
        scene.addLine(0, 0, model.canvas.width, model.canvas.height, QPen(Qt::white));
    }

    mutable int calls = 0;
};

trackview::RenderModel render_model()
{
    trackview::RenderModel model;
    model.canvas = {40, 20};
    model.tracks.push_back(
        {{1001}, {{2, 10}, {20, 10}}, {trackview::OccupancyState::Occupied}});
    model.switches.push_back(
        {{2001}, {{20, 10}, {30, 10}, {30, 15}},
         {trackview::OccupancyState::Free, trackview::SwitchPositionState::Divergent}});
    model.signal_items.push_back(
        {{3001}, {7, 8}, trackview::FacingDirection::TowardsB,
         {trackview::SignalIndicationState::Stop}});
    model.labels.push_back({"TEST", {16, 2}});
    return model;
}

int visible_red_pixels(const QImage& image)
{
    int count = 0;
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x)
        {
            const auto colour = image.pixelColor(x, y);
            if (colour.red() > 150 && colour.red() > colour.green() * 2)
                ++count;
        }
    return count;
}

TEST(QtSceneRenderer, CreatesInteractiveItemsWithInfrastructureIds)
{
    const trackview::EbiScreenTheme theme;
    const trackview::QtSceneRenderer renderer(theme);
    QGraphicsScene scene;
    renderer.render(render_model(), scene);
    ASSERT_FALSE(scene.items().empty());

    bool found_track = false;
    for (const auto* item : scene.items())
        found_track = found_track ||
                      item->data(trackview::kInfrastructureIdDataKey).toULongLong() == 1001;
    EXPECT_TRUE(found_track);
}

TEST(QtSceneRenderer, AcceptsSubstitutableThemeWithoutRendererChanges)
{
    const MagentaTheme theme;
    const trackview::QtSceneRenderer renderer(theme);
    QGraphicsScene scene;
    renderer.render(render_model(), scene);

    const QGraphicsPathItem* track_item = nullptr;
    for (const auto* item : scene.items())
        if (item->data(trackview::kInfrastructureIdDataKey).toULongLong() == 1001)
            track_item = qgraphicsitem_cast<const QGraphicsPathItem*>(item);
    ASSERT_NE(track_item, nullptr);
    EXPECT_EQ(track_item->pen().color(), QColor(Qt::magenta));
}

TEST(QtImageRenderer, DelegatesSceneCreationAndProducesVisibleStatePixels)
{
    const trackview::EbiScreenTheme theme;
    const trackview::QtSceneRenderer scene_renderer(theme);
    const trackview::QtImageRenderer image_renderer(scene_renderer);
    const auto image = image_renderer.render(render_model(), 4.0);
    EXPECT_FALSE(image.isNull());
    EXPECT_EQ(image.width(), 160);
    EXPECT_EQ(image.height(), 80);
    EXPECT_GT(visible_red_pixels(image), 10);
}

TEST(QtImageRenderer, DependsOnlyOnSceneRendererInterface)
{
    const StubSceneRenderer scene_renderer;
    const trackview::QtImageRenderer image_renderer(scene_renderer);
    EXPECT_FALSE(image_renderer.render(render_model(), 2.0).isNull());
    EXPECT_EQ(scene_renderer.calls, 1);
}

TEST(QtImageRenderer, RejectsInvalidScale)
{
    const trackview::EbiScreenTheme theme;
    const trackview::QtSceneRenderer scene_renderer(theme);
    const trackview::QtImageRenderer image_renderer(scene_renderer);
    EXPECT_THROW(static_cast<void>(image_renderer.render(render_model(), 0.0)),
                 std::invalid_argument);
}

}  // namespace
