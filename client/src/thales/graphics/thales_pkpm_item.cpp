#include "thales/graphics/thales_pkpm_item.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QTimer>
#include <cmath>

// ============================================================================
ThalesPkpmGraphic::ThalesPkpmGraphic(Direction dir, const QString& trackNum, QGraphicsItem* parent, engine::core::UID uid)
    : ThalesElementGraphic(parent, uid), m_dir(dir), m_trackNum(trackNum) {
    setAcceptHoverEvents(true);
}


void ThalesPkpmGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    ThalesElementGraphic::mousePressEvent(event);
}

QRectF ThalesPkpmGraphic::boundingRect() const {
    return QRectF(-6, -26, 50, 30);
}

void ThalesPkpmGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    // Trójkąt PKPM wg zrzutu ekranu znajduje się NAD torem (od y=-19 do y=-7, wysokość 12px)
    if (m_selected) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(160, 156, 0));
        painter->drawEllipse(QPointF(6, -13), 9.0, 9.0);
        painter->restore();
    }

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    // Żółty trójkąt PKPM (12px wysokości x 11px szerokości) umieszczony NAD torem
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(255, 255, 0));

    QPolygonF poly;
    if (m_dir == Right) {
        poly << QPointF(0, -19)
             << QPointF(11, -13)
             << QPointF(0, -7);
    } else {
        poly << QPointF(11, -19)
             << QPointF(0, -13)
             << QPointF(11, -7);
    }
    painter->drawPolygon(poly);

    // Etykieta numeru toru PKPM obok trójkąta (na tej samej wysokości, nie nachodzi na tor)
    if (!m_trackNum.isEmpty()) {
        QFont font("Arial", 8, QFont::Normal);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        painter->setPen(QColor(180, 180, 180));

        qreal textX = (m_dir == Right) ? -32 : 16;
        painter->drawText(QRectF(textX, -20, 28, 14), Qt::AlignVCenter | (m_dir == Right ? Qt::AlignRight : Qt::AlignLeft), m_trackNum);
    }
}

