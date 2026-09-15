#include "thales/graphics/thales_button_item.hpp"
#include <QGraphicsSceneMouseEvent>

ThalesButtonGraphic::ThalesButtonGraphic(const QString& text, Style style, bool hasArrow, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_text(text), m_style(style), m_hasArrow(hasArrow) {
    setAcceptHoverEvents(true);
}

QRectF ThalesButtonGraphic::boundingRect() const {
    QFont font("Consolas", 10, QFont::Bold);
    QFontMetrics fm(font);
    int textW = fm.horizontalAdvance(m_text);
    int totalW = textW + 20;
    if (m_hasArrow) {
        totalW += 12;
    }
    return QRectF(0, 0, totalW, 24);
}

void ThalesButtonGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_pressed = true;
        grabMouse();
        update();
        event->accept();
    } else {
        QGraphicsItem::mousePressEvent(event);
    }
}

void ThalesButtonGraphic::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_pressed = false;
        ungrabMouse();
        update();
        event->accept();
    } else {
        QGraphicsItem::mouseReleaseEvent(event);
    }
}

void ThalesButtonGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, false);

    QRectF r = boundingRect();

    QColor baseFill;
    QColor textCol;

    switch (m_style) {
        case BlueOT:
            baseFill = QColor(0, 32, 128);       // Granat #002080
            textCol = QColor(64, 128, 255);     // Jasnoniebieski #4080FF
            break;
        case GraySystem:
            baseFill = QColor(50, 50, 50);       // Stalowy szary
            textCol = QColor(180, 180, 180);    // Jasnoszary
            break;
        case RedPzb:
        default:
            baseFill = QColor(128, 0, 0);        // Bordo #800000
            textCol = QColor(255, 60, 60);      // Jasnoczerwony
            break;
    }

    // Gdy przycisk jest wciśnięty: delikatnie ściszony odcień z domieszką szarości i zapadnięta ramka 3D
    QColor fillCol = baseFill;
    if (m_pressed) {
        // Delikatnie ściszony odcień (domieszka szarości, ale zachowany główny kolor)
        fillCol = QColor(
            (baseFill.red() * 7 + 70 * 3) / 10,
            (baseFill.green() * 7 + 70 * 3) / 10,
            (baseFill.blue() * 7 + 70 * 3) / 10
        );
    }

    // Kolory fazowania krawędzi (przy wciśnięciu odwracamy ramki tworząc efekt zapadnięcia):
    QColor topLeftBevel = m_pressed ? QColor(0, 0, 0) : QColor(80, 80, 80);
    QColor bottomRightBevel = m_pressed ? QColor(80, 80, 80) : QColor(0, 0, 0);

    // 1. Wypełnienie wnętrza
    painter->fillRect(r.adjusted(2, 2, -2, -2), fillCol);

    // 2. Krawędź góra i lewo
    painter->setPen(QPen(topLeftBevel, 2));
    painter->drawLine(QPointF(1, 1), QPointF(r.width() - 1, 1));
    painter->drawLine(QPointF(1, 1), QPointF(1, r.height() - 1));

    // 3. Krawędź dół i prawo
    painter->setPen(QPen(bottomRightBevel, 2));
    painter->drawLine(QPointF(1, r.height() - 1), QPointF(r.width() - 1, r.height() - 1));
    painter->drawLine(QPointF(r.width() - 1, 1), QPointF(r.width() - 1, r.height() - 1));

    // 4. Tekst przycisku (Consolas, przy wciśnięciu przesunięty o 1px w dół i w prawo dając fizyczne zapadnięcie)
    QFont font("Consolas", 10, QFont::Bold);
    font.setStyleStrategy(QFont::NoAntialias);
    painter->setFont(font);
    painter->setPen(textCol);

    qreal offset = m_pressed ? 1.0 : 0.0;
    QRectF textRect = r.adjusted(4 + offset, 0 + offset, (m_hasArrow ? -14 : -4) + offset, 0 + offset);
    painter->drawText(textRect, Qt::AlignCenter, m_text);

    // 5. Opcjonalna strzałka ▶
    if (m_hasArrow) {
        painter->setBrush(textCol);
        painter->setPen(Qt::NoPen);
        qreal arrowX = r.width() - 11 + offset;
        qreal arrowY = r.height() / 2.0 + offset;
        QPolygonF arrow;
        arrow << QPointF(arrowX, arrowY - 4)
              << QPointF(arrowX + 5, arrowY)
              << QPointF(arrowX, arrowY + 4);
        painter->drawPolygon(arrow);
    }
}

    auto btnOtski = new ThalesButtonGraphic("OTSKI1", ThalesButtonGraphic::BlueOT);
    btnOtski->setPos(30, 48); m_scene->addItem(btnOtski);

    auto btnOteskx = new ThalesButtonGraphic("OTESKX", ThalesButtonGraphic::BlueOT);
    btnOteskx->setPos(115, 48); m_scene->addItem(btnOteskx);

    auto btnOtswa = new ThalesButtonGraphic("OTSWA", ThalesButtonGraphic::BlueOT);
    btnOtswa->setPos(200, 48); m_scene->addItem(btnOtswa);

    auto btnOtpoa = new ThalesButtonGraphic("OTPOA1", ThalesButtonGraphic::BlueOT, true);
    btnOtpoa->setPos(280, 48); m_scene->addItem(btnOtpoa);

    auto btnLoff = new ThalesButtonGraphic("LOFF", ThalesButtonGraphic::GraySystem);
    btnLoff->setPos(380, 48); m_scene->addItem(btnLoff);

    auto btnHmi = new ThalesButtonGraphic("HMI", ThalesButtonGraphic::GraySystem);
    btnHmi->setPos(445, 48); m_scene->addItem(btnHmi);

    auto btnPzb = new ThalesButtonGraphic("PZB", ThalesButtonGraphic::RedPzb);
    btnPzb->setPos(510, 48); m_scene->addItem(btnPzb);
