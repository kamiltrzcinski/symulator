#include "thales/graphics/thales_track_item.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QTimer>
#include <cmath>

// ============================================================================
ThalesTrackGraphic::ThalesTrackGraphic(qreal length, TrackState state, Termination term, const QString& trackNum, QGraphicsItem* parent, engine::core::UID uid)
    : ThalesElementGraphic(parent, uid), m_length(length), m_state(state), m_termination(term), m_trackNum(trackNum) {
    setAcceptHoverEvents(true);
}

QRectF ThalesTrackGraphic::boundingRect() const {
    return QRectF(-6, -24, m_length + 12, 40);
}

void ThalesTrackGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    ThalesElementGraphic::mousePressEvent(event);
}

void ThalesTrackGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, false);

    // 1. Wyraźny żółty pas zaznaczenia odcinka torowego (#A09C00 / RGB(160, 156, 0))
    // Wysokość 12px (od y=-6 do y=6), szerokość ~40px lub 70% długości toru, rysowany w tle
    if (m_selected) {
        qreal selW = qMin(42.0, m_length * 0.7);
        qreal selX = (m_length - selW) / 2.0;
        painter->fillRect(QRectF(selX, -6.0, selW, 12.0), QColor(160, 156, 0));
    }

    QColor col;
    switch (m_state) {
        case Free:             col = QColor(155, 155, 155); break; // Szary #9B9B9B
        case Occupied:         col = QColor(255, 0, 0);     break; // Czerwony #FF0000
        case RouteLockedTrain: col = QColor(0, 255, 0);     break; // Zieleń (#00FF00)
        case RouteLockedShunt: col = QColor(255, 255, 0);   break; // Żółty
        case MagentaRelease:   col = QColor(255, 0, 255);   break; // Magenta #FF00FF
        case PreReset:         col = QColor(139, 0, 0);     break; // Ciemnoczerwony #8B0000
        case FaultBlinking: {
            // Czerwono-biały migający: zmiana co 500ms
            bool blinkOn = (QDateTime::currentMSecsSinceEpoch() / 500) % 2 == 0;
            col = blinkOn ? QColor(255, 0, 0) : QColor(255, 255, 255);
            break;
        }
    }

    // 2. Rysowanie toru zasadniczego (grubość 4px: od y=-2 do y=2)
    QPen trackPen(col, 4, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    painter->setPen(trackPen);

    if (m_state == FaultBlinking) {
        // Cały odcinek miga: najpierw czerwony, potem biały (nie prążki!)
        bool blinkOn = (QDateTime::currentMSecsSinceEpoch() / 500) % 2 == 0;
        QColor flashCol = blinkOn ? QColor(255, 0, 0) : QColor(255, 255, 255);
        QPen flashPen(flashCol, 4, Qt::SolidLine, Qt::FlatCap);
        painter->setPen(flashPen);
        if (!m_trainNum.isEmpty()) {
            QFontMetrics fm(QFont("Consolas", 10, QFont::Normal));
            int tw = fm.horizontalAdvance(m_trainNum);
            qreal gapStart = (m_length - tw) / 2.0 - 4;
            qreal gapEnd = gapStart + tw + 8;
            painter->drawLine(QPointF(0, 0), QPointF(gapStart, 0));
            painter->drawLine(QPointF(gapEnd, 0), QPointF(m_length, 0));
        } else {
            painter->drawLine(QPointF(0, 0), QPointF(m_length, 0));
        }
    } else {
        if (!m_trainNum.isEmpty()) {
            QFontMetrics fm(QFont("Consolas", 10, QFont::Normal));
            int tw = fm.horizontalAdvance(m_trainNum);
            qreal gapStart = (m_length - tw) / 2.0 - 4;
            qreal gapEnd = gapStart + tw + 8;
            painter->drawLine(QPointF(0, 0), QPointF(gapStart, 0));
            painter->drawLine(QPointF(gapEnd, 0), QPointF(m_length, 0));
        } else {
            painter->drawLine(QPointF(0, 0), QPointF(m_length, 0));
        }
    }

    // Kozły oporowe: czysty prostokątny blok 4px x 12px
    if (m_termination == BufferStopLeft) {
        painter->fillRect(QRectF(0, -6, 4, 12), col);
    } else if (m_termination == BufferStopRight) {
        painter->fillRect(QRectF(m_length - 4, -6, 4, 12), col);
    }

    // 3. Numer toru umieszczony WYRAŹNIE NAD torem (y=-18 do y=-6, nie nachodzi na szynę)
    if (!m_trackNum.isEmpty()) {
        QFont font("Arial", 8, QFont::Normal);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        painter->setPen(QColor(180, 180, 180));
        painter->drawText(QRectF(0, -19, m_length, 14), Qt::AlignCenter, m_trackNum);
    }
    
    // Numer pociągu przecinający tor
    if (!m_trainNum.isEmpty()) {
        QFont font("Consolas", 10, QFont::Normal);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        
        // Draw text without background because we left a physical gap in the line
        painter->setPen(QColor(0, 255, 0)); // Green text
        painter->drawText(QRectF(0, -10, m_length, 20), Qt::AlignCenter, m_trainNum);
    }
}

