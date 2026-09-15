#include "thales/topology_loader.hpp"
#include "thales/graphics/thales_track_item.hpp"
#include "thales/graphics/thales_switch_item.hpp"
#include "thales/graphics/thales_signal_item.hpp"
#include <cmath>
#include <numbers>

bool TopologyLoader::loadFromRenderModel(const trackview::RenderModel& model, QGraphicsScene* scene) {
    if (!scene) return false;

    // Factory/Builder logic for Tracks
    for (const auto& track : model.tracks) {
        if (track.path.size() < 2) continue;
        
        // Use the first two points to determine length and rotation
        auto p1 = track.path.front();
        auto p2 = track.path.back();
        
        qreal dx = p2.x - p1.x;
        qreal dy = p2.y - p1.y;
        qreal length = std::sqrt(dx * dx + dy * dy);
        qreal angle = std::atan2(dy, dx) * 180.0 / std::numbers::pi;
        
        // Default state (will be updated via runtime state later)
        auto* item = new ThalesTrackGraphic(length, ThalesTrackGraphic::Free, ThalesTrackGraphic::None, "", engine::core::UID{track.id.value});
        item->setPos(p1.x, p1.y);
        item->setRotation(angle);
        scene->addItem(item);
    }

    // Factory logic for Switches
    for (const auto& sw : model.switches) {
        qreal dx = sw.ports.divergent.x - sw.ports.trunk.x;
        qreal dy = sw.ports.divergent.y - sw.ports.trunk.y;
        
        ThalesSwitchGraphic::BranchDir dir = ThalesSwitchGraphic::BranchDownRight;
        if (dx > 0 && dy < 0) dir = ThalesSwitchGraphic::BranchUpRight;
        else if (dx < 0 && dy < 0) dir = ThalesSwitchGraphic::BranchUpLeft;
        else if (dx < 0 && dy > 0) dir = ThalesSwitchGraphic::BranchDownLeft;
        else if (dx > 0 && dy > 0) dir = ThalesSwitchGraphic::BranchDownRight;
        
        auto* item = new ThalesSwitchGraphic(engine::core::UID{sw.id.value}, dir, false, ThalesSwitchGraphic::SwitchNormal);
        item->setPos(sw.ports.trunk.x, sw.ports.trunk.y);
        scene->addItem(item);
    }

    // Factory logic for Signals
    for (const auto& sig : model.signal_items) {
        ThalesSignalGraphic::SignalType type = (sig.facing == trackview::FacingDirection::TowardsB) 
                                             ? ThalesSignalGraphic::TrainRight 
                                             : ThalesSignalGraphic::TrainLeft;
                                             
        auto* item = new ThalesSignalGraphic("", type, ThalesSignalGraphic::Stop, true, engine::core::UID{sig.id.value});
        item->setPos(sig.position.x, sig.position.y);
        scene->addItem(item);
    }

    return true;
}
