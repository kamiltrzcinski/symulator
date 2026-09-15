#include "thales/graphics/thales_command_box_item.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QTimer>
#include <cmath>

// ============================================================================
ThalesCommandBoxGraphic::ThalesCommandBoxGraphic(qreal width, qreal height, QGraphicsItem* parent, engine::core::UID uid)
    : ThalesElementGraphic(parent, uid), m_width(width), m_height(height) {
}

QRectF ThalesCommandBoxGraphic::boundingRect() const {
    return QRectF(0, 0, m_width, m_height + 25);
}

void ThalesCommandBoxGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    QRectF boxRect(0, 0, m_width, m_height);

    // Cienka ramka 1px
    painter->setPen(QPen(QColor(100, 100, 100), 1));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boxRect.adjusted(0, 0, -1, -1));

    // Tekst w terminalu: łagodna czcionka Arial
    QFont font("Arial", 9, QFont::Normal);
    font.setStyleStrategy(QFont::PreferAntialias);
    painter->setFont(font);
    painter->setPen(QColor(180, 180, 180));

    painter->drawText(QPointF(12, 20), "WE :");
    painter->drawText(QPointF(12, 40), "KOM:");
    painter->drawText(QPointF(12, 60), "XX :");

    // Klawisze funkcyjne pod terminalem
    painter->setPen(QColor(255, 60, 60));
    painter->drawText(QPointF(12, m_height + 18), "F1   F2");
}

