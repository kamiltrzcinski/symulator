#include "thales/graphics/thales_element_graphic.hpp"

ThalesElementGraphic::ThalesElementGraphic(QGraphicsItem* parent, engine::core::UID uid)
    : QGraphicsObject(parent), m_uid(uid) {
    setAcceptHoverEvents(true);
}

void ThalesElementGraphic::setSelectedState(bool selected) {
    if (m_selected != selected) {
        m_selected = selected;
        update();
    }
}

void ThalesElementGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_uid);
    }
    QGraphicsObject::mousePressEvent(event);
}
