#include <trackview/qt/scene_renderer.hpp>

#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPath>
#include <QVariant>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace trackview
{
namespace
{

QPainterPath to_qt_path(const Path& path)
{
    QPainterPath result;
    if (path.empty())
        return result;
    result.moveTo(path.front().x, path.front().y);
    for (std::size_t index = 1; index < path.size(); ++index)
        result.lineTo(path[index].x, path[index].y);
    return result;
}

void attach_id(QGraphicsItem& item, InfrastructureId id)
{
    item.setData(kInfrastructureIdDataKey,
                 QVariant::fromValue<qulonglong>(id.value));
}

}  // namespace

void QtSceneRenderer::render(const RenderModel& model, QGraphicsScene& scene) const
{
    render(model, scene, true);
}

void QtSceneRenderer::render(const RenderModel& model, QGraphicsScene& scene, bool interactive) const
{
    scene.clear();
    scene.setSceneRect(0.0, 0.0, model.canvas.width, model.canvas.height);
    scene.setBackgroundBrush(theme_.background());

    for (const auto& track : model.tracks)
    {
        auto* item = scene.addPath(to_qt_path(track.path),
                                   theme_.track_pen(track.state.occupancy));
        attach_id(*item, track.id);
        item->setFlag(QGraphicsItem::ItemIsSelectable, interactive);
    }

    for (const auto& switch_item : model.switches)
    {
        const bool straight_active =
            switch_item.state.position == SwitchPositionState::Straight;
        const bool divergent_active =
            switch_item.state.position == SwitchPositionState::Divergent;
        auto* straight = scene.addLine(
            switch_item.ports.trunk.x, switch_item.ports.trunk.y,
            switch_item.ports.straight.x, switch_item.ports.straight.y,
            theme_.switch_pen(switch_item.state.occupancy, straight_active));
        auto* divergent = scene.addLine(
            switch_item.ports.trunk.x, switch_item.ports.trunk.y,
            switch_item.ports.divergent.x, switch_item.ports.divergent.y,
            theme_.switch_pen(switch_item.state.occupancy, divergent_active));
        attach_id(*straight, switch_item.id);
        attach_id(*divergent, switch_item.id);
        straight->setFlag(QGraphicsItem::ItemIsSelectable, interactive);
        divergent->setFlag(QGraphicsItem::ItemIsSelectable, interactive);
    }

    for (const auto& signal_item : model.signal_items)
    {
        auto* item = scene.addEllipse(
            signal_item.position.x - 1.1, signal_item.position.y - 1.1, 2.2, 2.2,
            QPen(Qt::black, 0.4), theme_.signal_brush(signal_item.state.indication));
        attach_id(*item, signal_item.id);
        item->setFlag(QGraphicsItem::ItemIsSelectable, interactive);
    }

    for (const auto& label : model.labels)
    {
        auto* item = scene.addText(QString::fromStdString(label.text));
        item->setDefaultTextColor(theme_.label_colour());
        item->setScale(0.12);
        item->setPos(label.anchor.x, label.anchor.y);
        item->setFlag(QGraphicsItem::ItemIsSelectable, false);
    }
}

QImage QtImageRenderer::render(const RenderModel& model, double scale) const
{
    if (!std::isfinite(scale) || scale <= 0.0)
        throw std::invalid_argument("image_renderer: scale must be finite and positive");
    QGraphicsScene scene;
    scene_renderer_.render(model, scene);
    const int width =
        std::max(1, static_cast<int>(std::ceil(model.canvas.width * scale)));
    const int height =
        std::max(1, static_cast<int>(std::ceil(model.canvas.height * scale)));
    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(scene.backgroundBrush().color());
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    scene.render(&painter, QRectF(0, 0, width, height), scene.sceneRect());
    return image;
}

}  // namespace trackview
