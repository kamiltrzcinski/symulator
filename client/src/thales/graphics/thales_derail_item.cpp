#include "thales/graphics/thales_derail_item.hpp"
#include <QGraphicsSceneMouseEvent>

ThalesDerailGraphic::ThalesDerailGraphic(const QString& name, Direction dir, DerailState state, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_name(name), m_dir(dir), m_state(state) {
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
    QGraphicsItem::mousePressEvent(event);
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

    auto derail1 = new ThalesDerailGraphic("D1", ThalesDerailGraphic::Right, ThalesDerailGraphic::Placed); derail1->setPos(50, derY); m_scene->addItem(derail1);
    auto lblDer1 = new ThalesLabelGraphic("Nałożona (Wolna)", QColor(155,155,155)); lblDer1->setPos(50, derY+15); m_scene->addItem(lblDer1);

    auto derail2 = new ThalesDerailGraphic("D2", ThalesDerailGraphic::Right, ThalesDerailGraphic::Clear); derail2->setPos(170, derY); m_scene->addItem(derail2);
    auto lblDer2 = new ThalesLabelGraphic("Zdjęta (Wolna)", QColor(155,155,155)); lblDer2->setPos(170, derY+15); m_scene->addItem(lblDer2);

    auto derail3 = new ThalesDerailGraphic("D3", ThalesDerailGraphic::Right, ThalesDerailGraphic::NoControl); derail3->setPos(290, derY); m_scene->addItem(derail3);
    auto lblDer3 = new ThalesLabelGraphic("Brak Kontroli (Miga)", QColor(255,255,255)); lblDer3->setPos(290, derY+15); m_scene->addItem(lblDer3);

    auto derail4 = new ThalesDerailGraphic("D4", ThalesDerailGraphic::Right, ThalesDerailGraphic::Stopped); derail4->setPos(410, derY); m_scene->addItem(derail4);
    auto lblDer4 = new ThalesLabelGraphic("Zastopowana", QColor(255,0,255)); lblDer4->setPos(410, derY+15); m_scene->addItem(lblDer4);

    auto derail5 = new ThalesDerailGraphic("D5", ThalesDerailGraphic::Right, ThalesDerailGraphic::Clear); derail5->setOccupied(true); derail5->setPos(530, derY); m_scene->addItem(derail5);
    auto lblDer5 = new ThalesLabelGraphic("Zdjeta (Zajeta)", QColor(255,0,0)); lblDer5->setPos(530, derY+15); m_scene->addItem(lblDer5);
