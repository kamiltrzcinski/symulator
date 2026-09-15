#pragma once

#include <QGraphicsScene>
#include <QString>
#include <memory>
#include <trackview/core/render_model.hpp>

class TopologyLoader {
public:
    // Loads the topology from a RenderModel and populates the scene
    static bool loadFromRenderModel(const trackview::RenderModel& model, QGraphicsScene* scene);
};
