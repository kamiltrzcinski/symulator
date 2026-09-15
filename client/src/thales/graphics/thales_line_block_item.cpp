#include "thales/graphics/thales_line_block_item.hpp"
#include <QGraphicsSceneMouseEvent>

ThalesLineBlockGraphic::ThalesLineBlockGraphic(State state, const QString& label, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_state(state), m_label(label) {
    setAcceptHoverEvents(true);
}

QRectF ThalesLineBlockGraphic::boundingRect() const {
    return QRectF(0, -18, 100, 36);
}

void ThalesLineBlockGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    QGraphicsItem::mousePressEvent(event);
}

void ThalesLineBlockGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    if (m_selected) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(160, 156, 0));
        // Elipsa zaznaczenia
        painter->drawEllipse(QPointF(50, -6), 30.0, 14.0);
        painter->restore();
    }

    painter->save();
    
    // Przesuwamy i skalujemy, by symbol 118x30 idealnie wpasował się na środek
    painter->translate(29, -11);
    painter->scale(0.35, 0.35);

    QColor yellow(255, 255, 0);
    QColor grey(155, 155, 155);

    // Kawałki toru po bokach (jak na zrzucie ekranu)
    painter->setPen(Qt::NoPen);
    painter->setBrush(grey);
    painter->drawRect(QRectF(-20, 13, 20, 4));
    painter->drawRect(QRectF(118, 13, 20, 4));

    auto drawArrowH = [&](qreal x, bool left, QColor c) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->translate(x, 0);
        if (!left) {
            painter->translate(41, 0);
            painter->scale(-1, 1);
        }
        painter->drawRect(QRectF(33, 8, 8, 13)); // Stem
        painter->drawRect(QRectF(25, 0, 8, 30));
        painter->drawRect(QRectF(21, 2, 12, 26));
        painter->drawRect(QRectF(17, 4, 16, 22));
        painter->drawRect(QRectF(12, 6, 21, 18));
        painter->drawRect(QRectF(8, 9, 25, 12));
        painter->drawRect(QRectF(3, 11, 30, 8));
        painter->drawRect(QRectF(0, 13, 33, 4));
        painter->restore();
    };

    auto drawBar = [&](qreal x, qreal w, QColor c) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(c);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->drawRect(QRectF(x, 8, w, 14));
        painter->restore();
    };

    bool blinkOn2 = (QDateTime::currentMSecsSinceEpoch() / 500) % 2 == 0;
    QColor red(255, 0, 0);

    if (m_state == Receiving) {
        drawArrowH(0, true, yellow);
        drawBar(47, 71, yellow);
    } else if (m_state == Sending) {
        drawBar(0, 71, yellow);
        drawArrowH(77, false, yellow);
    } else if (m_state == PermissionRequested) {
        // "strzałka zwrócona grotem w lewą stronę jest tak naprawdę prostokątem ... ta część z grotem za prostokątem mryga"
        drawBar(0, 71, yellow); // it is currently in Sending state (so it is yellow)
        if (blinkOn2) {
            drawArrowH(77, false, yellow); // arrow part blinks
        }
    } else if (m_state == EmergencyChange) {
        // Migająca czerwona strzałka (awaryjna zmiana kierunku)
        if (blinkOn2) {
            drawArrowH(0, true, red);
            drawArrowH(77, false, red);
        } else {
            drawBar(0, 118, red);
        }
    } else { // Neutral
        drawArrowH(0, true, grey);
        drawBar(47, 24, grey);
        drawArrowH(77, false, grey);
    }

    painter->restore();

    if (!m_label.isEmpty()) {
        painter->setRenderHint(QPainter::TextAntialiasing, true);
        QFont font("Arial", 8, QFont::Bold);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        painter->setPen(QColor(135, 155, 175)); // lekko niebieskawy szary
        painter->drawText(QRectF(0, -32, 100, 15), Qt::AlignCenter | Qt::AlignBottom, m_label);
    }
}

    auto blk1 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Neutral, "Neutral"); blk1->setPos(30, blkY); m_scene->addItem(blk1);
    auto blk2 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Sending, "Sending"); blk2->setPos(130, blkY); m_scene->addItem(blk2);

    auto blk3 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Receiving, "Receiving"); blk3->setPos(230, blkY); m_scene->addItem(blk3);
    auto blk4 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::PermissionRequested, "PermReq(Miga)"); blk4->setPos(330, blkY); m_scene->addItem(blk4);

    auto blk5 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::EmergencyChange, "Emerg(Miga)"); blk5->setPos(430, blkY); m_scene->addItem(blk5);

