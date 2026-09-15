#include "thales/graphics/thales_signal_box_item.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QTimer>
#include <cmath>

// ============================================================================
ThalesSignalBoxGraphic::ThalesSignalBoxGraphic(const QString& label, QGraphicsItem* parent, engine::core::UID uid)
    : ThalesElementGraphic(parent, uid), m_label(label) {
    setAcceptHoverEvents(true);
}

QRectF ThalesSignalBoxGraphic::boundingRect() const {
    return QRectF(-5, -5, 46, 60);
}

void ThalesSignalBoxGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    ThalesElementGraphic::mousePressEvent(event);
}

void ThalesSignalBoxGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    if (m_selected) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(160, 156, 0));
        // Koło dopasowane do zmniejszonego symbolu
        painter->drawEllipse(QPointF(18, 12), 22.0, 22.0);
        painter->restore();
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    
    // Skalujemy o połowę (36x25 px zamiast 72x50 px)
    painter->scale(0.5, 0.5);

    QPainterPath path;
    path.addRect(0, 0, 72, 50);

    QPainterPath holes;
    holes.addRect(11, 11, 49, 7);

    holes.addRect(32, 24, 7, 2);
    holes.addRect(31, 26, 9, 5);
    holes.addRect(32, 31, 7, 2);

    holes.addRect(13, 30, 2, 4);
    holes.addRect(55, 30, 2, 4);
    
    holes.addRect(13, 33, 6, 1);
    holes.addRect(51, 33, 6, 1);
    holes.addRect(17, 34, 2, 2);
    holes.addRect(51, 34, 2, 2);
    
    holes.addRect(17, 35, 7, 1);
    holes.addRect(46, 35, 7, 1);
    holes.addRect(22, 36, 2, 2);
    holes.addRect(46, 36, 2, 2);
    
    holes.addRect(22, 37, 26, 2);

    // Wycinamy otwory, by było widać tło i ew. żółte halo zaznaczenia
    QPainterPath finalPath = path.subtracted(holes);
    
    painter->setPen(Qt::NoPen);
    painter->fillPath(finalPath, QColor(155, 155, 155));

    painter->restore();

    // Etykieta (tekst bez skali)
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    QFont font("Arial", 8, QFont::Normal);
    font.setStyleStrategy(QFont::PreferAntialias);
    painter->setFont(font);
    painter->setPen(QColor(180, 180, 180));
    painter->drawText(QRectF(0, 28, 36, 20), Qt::AlignCenter | Qt::AlignTop, m_label);
}


