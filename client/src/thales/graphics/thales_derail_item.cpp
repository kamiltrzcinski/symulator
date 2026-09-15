#include "thales/graphics/thales_derail_item.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QTimer>
#include <cmath>

// ============================================================================
ThalesDerailGraphic::ThalesDerailGraphic(const QString& name, Direction dir, DerailState state, QGraphicsItem* parent, engine::core::UID uid)
    : ThalesElementGraphic(parent, uid), m_name(name), m_dir(dir), m_state(state) {
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF ThalesDerailGraphic::boundingRect() const {
    return QRectF(-15, -35, 60, 50);
}

void ThalesDerailGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    ThalesElementGraphic::mousePressEvent(event);
}

void ThalesDerailGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, false);

    bool blinkOn = (QDateTime::currentMSecsSinceEpoch() / 500) % 2 == 0;

    QColor trackCol = m_occupied ? QColor(255, 0, 0) : QColor(155, 155, 155);
    QColor symCol = trackCol;

    if (m_state == Stopped) {
        symCol = QColor(255, 0, 255);
        trackCol = symCol;
    }

    // NoControl: tor rysowany ZAWSZE w kolorze bazy, symbol (prostokąt) tylko miga
    bool drawSymbol = true;
    if (m_state == NoControl) {
        symCol = QColor(255, 255, 255);
        drawSymbol = blinkOn;  // segment miga, tor stały
        trackCol = m_occupied ? QColor(255, 0, 0) : QColor(155, 155, 155); // tor bez zmian
    }

    QPen trackPen(trackCol, 4, Qt::SolidLine, Qt::FlatCap);
    painter->setPen(trackPen);

    if (m_selected) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(160, 156, 0));
        painter->drawEllipse(QPointF(10, 0), 16.0, 16.0);
        painter->setPen(trackPen);
    }

    // "Wykolejnica w zmienionym położeniu otwiera się w prawą stronę, czyli cały prostokąt rotuje się, a przerwa w torze jest scalona."
    if (m_state == Placed || m_state == Stopped || m_state == NoControl) {
        // Track has a gap (always drawn)
        painter->drawLine(QPointF(-10, 0), QPointF(5, 0));
        painter->drawLine(QPointF(15, 0), QPointF(30, 0));
        
        // Vertical rectangle in the gap - only if drawSymbol (for NoControl: blinks)
        if (drawSymbol) {
            painter->fillRect(QRectF(8, -8, 4, 16), symCol);
        }
    } else if (m_state == Clear) {
        // Gap is merged, track is continuous.
        painter->drawLine(QPointF(-10, 0), QPointF(5, 0));
        painter->drawLine(QPointF(15, 0), QPointF(30, 0));
        
        // Two horizontal rectangles in the middle
        painter->fillRect(QRectF(6, -2, 8, 4), symCol); // Fills the gap
        painter->fillRect(QRectF(6, -10, 8, 4), symCol); // Above the gap
    }

    // Name
    if (!m_name.isEmpty()) {
        QFont font("Consolas", 10, QFont::Bold);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        painter->setPen(QColor(130, 130, 130));
        painter->drawText(QRectF(-15, -30, 50, 14), Qt::AlignCenter, m_name);
    }
}

