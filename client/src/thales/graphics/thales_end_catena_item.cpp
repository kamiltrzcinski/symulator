#include "thales/graphics/thales_end_catena_item.hpp"
#include <QGraphicsSceneMouseEvent>

ThalesEndCatenaGraphic::ThalesEndCatenaGraphic(const QString& trackNum, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_trackNum(trackNum) {
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::NoButton);
}

QRectF ThalesEndCatenaGraphic::boundingRect() const {
    return QRectF(-6, -26, 45, 30);
}

void ThalesEndCatenaGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    QGraphicsItem::mousePressEvent(event);
}

void ThalesEndCatenaGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    if (m_selected) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(160, 156, 0));
        painter->drawEllipse(QPointF(5, -13), 9.0, 9.0);
        painter->restore();
    }

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    QColor redCol(255, 0, 0);
    QPen pen(redCol, 2, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
    painter->setPen(pen);

    // Czerwona strzałka końca trakcji ↙ umieszczona NAD torem (y=-20 do y=-8)
    painter->drawLine(QPointF(4, -20), QPointF(4, -16));
    painter->drawLine(QPointF(4, -16), QPointF(0, -12));
    painter->drawLine(QPointF(0, -12), QPointF(0, -8));

    painter->drawLine(QPointF(0, -8), QPointF(-3, -11));
    painter->drawLine(QPointF(0, -8), QPointF(3, -11));

    if (!m_trackNum.isEmpty()) {
        QFont font("Arial", 8, QFont::Normal);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        painter->setPen(QColor(180, 180, 180));
        painter->drawText(QRectF(8, -19, 25, 14), Qt::AlignVCenter | Qt::AlignLeft, m_trackNum);
    }
}
