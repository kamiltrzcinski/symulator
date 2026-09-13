#include "thales/thales_browser.hpp"

#include <QFont>
#include <QPolygonF>
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QImage>
#include <QDateTime>
#include <QTimer>



// ============================================================================
// 1. ThalesButtonGraphic (Rysowany w 100% kodem, ostre krawędzie 3D, Consolas)
// ============================================================================
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


// ============================================================================
// 2. ThalesTrackGraphic (Tor 4px, animacja zaznaczenia)
// ============================================================================
ThalesTrackGraphic::ThalesTrackGraphic(qreal length, TrackState state, Termination term, const QString& trackNum, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_length(length), m_state(state), m_termination(term), m_trackNum(trackNum) {
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
    QGraphicsItem::mousePressEvent(event);
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


// ============================================================================
// 3. ThalesSignalGraphic (Semafor półsamoczynny + tarcza manewrowa)
// ============================================================================
ThalesSignalGraphic::ThalesSignalGraphic(const QString& name, SignalType type, SignalState state, bool labelAbove, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_name(name), m_type(type), m_state(state), m_labelAbove(labelAbove) {
    setAcceptHoverEvents(true);
    setZValue(10);
}

QRectF ThalesSignalGraphic::boundingRect() const {
    return QRectF(-15, -28, 90, 56);
}

QPainterPath ThalesSignalGraphic::shape() const {
    QPainterPath path;
    const qreal sW   = 12.8;
    const qreal oW   = 13.6;
    const qreal kGap = 2.0;
    qreal totalW = 0;
    
    if (m_type == TrainLeft || m_type == TrainRight) totalW = sW;
    else if (m_type == ShuntLeft || m_type == ShuntRight) totalW = oW;
    else totalW = sW + kGap + oW;
    
    // Tight bounding box around the actual signal shape
    path.addRect(0, -10, totalW, 20);
    return path;
}

void ThalesSignalGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    QGraphicsItem::mousePressEvent(event);
}


// ============================================================================
// Solid train semaphore – filled triangle, vector-only, NO antialiasing.
// Proportions scaled up ~1.33x.
// ============================================================================
static void drawSolidSemaphore(QPainter* painter, qreal cx, qreal cy, bool pointsLeft, const QColor& color) {
    const qreal hw = 6.4;   // half width (tip → base)
    const qreal ht = 6.5;   // half height (base spans ±ht)
    const qreal ty = -0.6;  // tip is slightly above centre
    QPolygonF tri;
    if (pointsLeft) {
        tri << QPointF(cx - hw, cy + ty)   // tip (leftmost)
            << QPointF(cx + hw, cy - ht)   // base top-right
            << QPointF(cx + hw, cy + ht);  // base bottom-right
    } else {
        tri << QPointF(cx + hw, cy + ty)   // tip (rightmost)
            << QPointF(cx - hw, cy - ht)   // base top-left
            << QPointF(cx - hw, cy + ht);  // base bottom-left
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPolygon(tri);
}

// ============================================================================
// Outline shunt semaphore – hollow chevron arrow, vector-only, NO antialiasing.
// Proportions scaled up ~1.33x.
// ============================================================================
static void drawOutlineSemaphore(QPainter* painter, qreal cx, qreal cy, bool pointsLeft, const QColor& color) {
    // 8-point hollow left-pointing chevron (manoeuvring / shunting signal)
    static const qreal pts[8][2] = {
        { 6.00,  7.33},
        {-6.81,  0.24},
        { 6.81, -7.33},
        { 6.81, -3.84},
        { 0.76, -0.47},
        {-0.52,  0.24},
        { 6.46,  4.19},
        { 6.46,  7.45},
    };
    QPolygonF poly;
    for (const auto& p : pts) {
        const qreal x = pointsLeft ? p[0] : -p[0];
        poly << QPointF(cx + x, cy + p[1]);
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawPolygon(poly);
}

void ThalesSignalGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    // Pixel-perfect, no antialiasing – authentic Thales ML8 display look
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, false);

    // Route state → signal colour (Match Free track colour for Stop)
    bool blinkOn = (QDateTime::currentMSecsSinceEpoch() / 500) % 2 == 0;
    QColor signalColor(155, 155, 155);            // stop / grey (matches Free track)
    if (m_state == ProceedTrain)       signalColor = QColor(0, 255, 0);      // green
    else if (m_state == ProceedShunt)  signalColor = QColor(255, 255, 0);    // yellow
    else if (m_state == SignalStopped) signalColor = QColor(255, 0, 255);    // magenta
    else if (m_state == Substitute)    signalColor = blinkOn ? QColor(255, 255, 255) : QColor(33, 33, 33); // migający biały

    // Element widths (scaled up ~1.33x)
    const qreal sW   = 12.8;  // solid (train) semaphore width
    const qreal oW   = 13.6;  // outline (shunt) semaphore width
    const qreal kGap = 2.0;   // gap between compound elements

    qreal totalW = 0;
    bool hasSolid = false, hasOutline = false, solidFirst = false;
    switch (m_type) {
        case TrainLeft:
        case TrainRight:
            totalW = sW;             hasSolid = true;                      break;
        case ShuntLeft:
        case ShuntRight:
            totalW = oW;             hasOutline = true;                     break;
        case TrainAndShuntLeft:
            totalW = sW + kGap + oW; hasSolid = hasOutline = solidFirst = true; break;
        case TrainAndShuntRight:
            totalW = oW + kGap + sW; hasSolid = hasOutline = true;         break;
    }

    const bool pointsLeft = (m_type == TrainLeft || m_type == ShuntLeft || m_type == TrainAndShuntLeft);

    // ── Selection halo or track cutout ──────────────────────────────────────
    if (m_selected) {
        // Jednolite koło (nie owal) wg screena – promień dopasowany do szerokości sygnału
        const qreal r = qMax(totalW / 2.0 + 4.0, 9.0);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(160, 156, 0));
        painter->drawEllipse(QPointF(totalW / 2.0, 0.0), r, r);
    } else {
        // Erase track stripe completely behind signal body
        painter->fillRect(QRectF(0.0, -6.0, totalW, 12.0), QColor(33, 33, 33));

        // Zapadanie pikseli (szyjka): na 1px przed i za semaforem, wycinamy górne
        // i dolne piksele toru (tor ma y od -2 do 1). Wytniemy wszystko powyżej
        // y=-1 i poniżej y=0 (zostają tylko 2 piksele na środku)
        
        // Lewa strona (x = -1)
        painter->fillRect(QRectF(-1.0, -6.0, 1.0, 5.0), QColor(33, 33, 33)); // wycina y < -1
        painter->fillRect(QRectF(-1.0,  1.0, 1.0, 5.0), QColor(33, 33, 33)); // wycina y >= 1

        // Prawa strona (x = totalW)
        painter->fillRect(QRectF(totalW, -6.0, 1.0, 5.0), QColor(33, 33, 33)); // wycina y < -1
        painter->fillRect(QRectF(totalW,  1.0, 1.0, 5.0), QColor(33, 33, 33)); // wycina y >= 1
    }

    // ── Signal shapes ────────────────────────────────────────────────────────
    qreal solidCX = 0.0, outlineCX = 0.0;
    if (hasSolid && !hasOutline) {
        solidCX = sW / 2.0;
    } else if (hasOutline && !hasSolid) {
        outlineCX = oW / 2.0;
    } else if (solidFirst) {           // TrainAndShuntLeft: solid left, outline right
        solidCX   = sW / 2.0;
        outlineCX = sW + kGap + oW / 2.0;
    } else {                           // TrainAndShuntRight: outline left, solid right
        outlineCX = oW / 2.0;
        solidCX   = oW + kGap + sW / 2.0;
    }

    if (hasSolid)   drawSolidSemaphore  (painter, solidCX,   0.0, pointsLeft, signalColor);
    if (hasOutline) drawOutlineSemaphore (painter, outlineCX, 0.0, pointsLeft, signalColor);

    // ── Label ────────────────────────────────────────────────────────────────
    if (!m_name.isEmpty()) {
        painter->setRenderHint(QPainter::TextAntialiasing, true);
        QFont font("Arial", 8, QFont::Normal);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        painter->setPen(QColor(180, 180, 180));
        const qreal textY = m_labelAbove ? -22 : 12;
        painter->drawText(QRectF(-15, textY, totalW + 30, 14), Qt::AlignCenter, m_name);
    }
}



// ============================================================================
// 4. ThalesPkpmGraphic (Żółty trójkąt, większy proporcjonalnie ~12x12 px)
// ============================================================================
ThalesPkpmGraphic::ThalesPkpmGraphic(Direction dir, const QString& trackNum, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_dir(dir), m_trackNum(trackNum) {
    setAcceptHoverEvents(true);
}


void ThalesPkpmGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    QGraphicsItem::mousePressEvent(event);
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


// ============================================================================
// 5. ThalesEndCatenaGraphic (Koniec Sieci Trakcyjnej - Czerwona Strzałka ↙)
// ============================================================================
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


// ============================================================================
// 6. ThalesSwitchGraphic (Stromy kąt ok. 70° od poziomu / 20° od pionu)
// ============================================================================
ThalesSwitchGraphic::ThalesSwitchGraphic(const QString& name, BranchDir dir, bool divergingOccupied, SwitchState state, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_name(name), m_dir(dir), m_divergingOccupied(divergingOccupied), m_switchState(state) {
    setAcceptHoverEvents(true);
}

QRectF ThalesSwitchGraphic::boundingRect() const {
    return QRectF(-5, -20, 80, 40);
}

void ThalesSwitchGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    QGraphicsItem::mousePressEvent(event);
}

void ThalesSwitchGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, false);

    bool blinkOn = (QDateTime::currentMSecsSinceEpoch() / 500) % 2 == 0;

    // Layout: two horizontal rails at y=-11 (top) and y=+11 (bottom)
    // The blade goes diagonally from (rootX, rootY) to (tipX, tipY)
    // Reference image: blade angle ~60° from horizontal, clear pixel edges
    const qreal trackY1 = -11.0;  // top rail Y
    const qreal trackY2 = +11.0;  // bottom rail Y
    const qreal trackLen = 70.0;
    
    qreal rootX, rootY, tipX, tipY;
    bool rootOnBottom = (m_dir == BranchUpRight || m_dir == BranchUpLeft);
    
    if (m_dir == BranchUpRight) {
        rootX = 18.0;  rootY = trackY2;
        tipX  = 52.0;  tipY  = trackY1;
    } else if (m_dir == BranchDownRight) {
        rootX = 18.0;  rootY = trackY1;
        tipX  = 52.0;  tipY  = trackY2;
    } else if (m_dir == BranchUpLeft) {
        rootX = 52.0;  rootY = trackY2;
        tipX  = 18.0;  tipY  = trackY1;
    } else { // BranchDownLeft
        rootX = 52.0;  rootY = trackY1;
        tipX  = 18.0;  tipY  = trackY2;
    }

    qreal dx = tipX - rootX;
    qreal dy = tipY - rootY;
    qreal L = std::hypot(dx, dy);
    qreal ux = dx / L;
    qreal uy = dy / L;
    
    // 1px gap from root rail, 1px gap before tip rail
    QPointF bladeStart(rootX + ux * 2.0, rootY + uy * 2.0);
    QPointF bladeEnd(tipX - ux * 2.0, tipY - uy * 2.0);
    QPointF bladeCenter((rootX + tipX) / 2.0, (rootY + tipY) / 2.0);

    // ---- Selection oval along blade ----
    if (m_selected) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(160, 156, 0, 180));
        painter->translate(bladeCenter);
        qreal angle = std::atan2(dy, dx) * 180.0 / M_PI;
        painter->rotate(angle);
        painter->drawEllipse(QRectF(-L / 2.0 - 2, -9, L + 4, 18));
        painter->restore();
    }

    // ---- State colors ----
    QColor straightCol = QColor(155, 155, 155);
    if (m_switchState == SwitchStopped && !m_diverging)
        straightCol = QColor(255, 0, 255);
    else if (m_switchState == SwitchStopped && m_diverging)
        straightCol = QColor(155, 155, 155);

    bool drawBlade = true;
    QColor bladeCol = QColor(155, 155, 155);

    if (m_switchState == SwitchStopped) {
        bladeCol = m_diverging ? QColor(255, 0, 255) : QColor(155, 155, 155);
    } else if (m_switchState == SwitchNoControl) {
        drawBlade = blinkOn;
        bladeCol = QColor(255, 255, 255);
    } else if (m_switchState == SwitchDerailed) {
        drawBlade = blinkOn;
        bladeCol = QColor(255, 0, 0);
    }

    // Active rail = the one the blade originates from
    QColor activeRailCol = m_diverging ? straightCol : (m_divergingOccupied ? QColor(255, 0, 0) : straightCol);
    QColor inactiveRailCol = QColor(155, 155, 155);
    
    // Top rail
    QPen topPen(rootOnBottom ? inactiveRailCol : activeRailCol, 4, Qt::SolidLine, Qt::FlatCap);
    painter->setPen(topPen);
    painter->drawLine(QPointF(0, trackY1), QPointF(trackLen, trackY1));

    // Bottom rail  
    QPen botPen(rootOnBottom ? activeRailCol : inactiveRailCol, 4, Qt::SolidLine, Qt::FlatCap);
    painter->setPen(botPen);
    painter->drawLine(QPointF(0, trackY2), QPointF(trackLen, trackY2));

    // ---- Blade ----
    if (drawBlade) {
        QPen bladePen(bladeCol, 4, Qt::SolidLine, Qt::FlatCap);
        painter->setPen(bladePen);
        painter->drawLine(bladeStart, bladeEnd);
    }

    // ---- Name label ----
    if (!m_name.isEmpty()) {
        QFont font("Consolas", 8, QFont::Normal);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        painter->setPen(QColor(180, 180, 180));
        painter->drawText(QRectF(0, -24, 40, 12), Qt::AlignLeft | Qt::AlignVCenter, m_name);
    }
}


// ============================================================================
// 7. ThalesPlatformGraphic (Peron: Podwójna linia górna i dolna z tekstem w środku)
// Perony NIE podlegają zaznaczaniu myszą.
// ============================================================================
ThalesPlatformGraphic::ThalesPlatformGraphic(const QString& text, qreal width, qreal height, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_text(text), m_width(width), m_height(height) {
}

QRectF ThalesPlatformGraphic::boundingRect() const {
    return QRectF(0, 0, m_width, m_height);
}

void ThalesPlatformGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    QRectF r(0, 0, m_width, m_height);

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    // Czyste tło pulpitu wewnątrz peronu
    painter->fillRect(r, QColor(33, 33, 33));

    QPen borderPen(QColor(155, 155, 155), 1);
    painter->setPen(borderPen);

    // Podwójna linia na górze peronu (odstęp 1px, czyli linie obok siebie z jednym pikselem przerwy)
    painter->drawLine(QPointF(0, 0), QPointF(m_width - 1, 0));
    painter->drawLine(QPointF(0, 2), QPointF(m_width - 1, 2));

    // Podwójna linia na dole peronu (odstęp 1px)
    painter->drawLine(QPointF(0, m_height - 1), QPointF(m_width - 1, m_height - 1));
    painter->drawLine(QPointF(0, m_height - 3), QPointF(m_width - 1, m_height - 3));

    // Pionowe zamknięcia po bokach
    painter->drawLine(QPointF(0, 0), QPointF(0, m_height - 1));
    painter->drawLine(QPointF(m_width - 1, 0), QPointF(m_width - 1, m_height - 1));

    // Tekst peronu dokładnie wycentrowany w ramce (odcień niebieskawy wg wzoru)
    QFont font("Arial", 8, QFont::Normal);
    font.setStyleStrategy(QFont::PreferAntialias);
    painter->setFont(font);
    painter->setPen(QColor(135, 155, 175)); // chabrowo-szary
    painter->drawText(r, Qt::AlignCenter, m_text);
}


// ============================================================================
// 8. ThalesCommandBoxGraphic (Terminal poleceń WE: KOM: XX: - czcionka Arial)
// ============================================================================
ThalesCommandBoxGraphic::ThalesCommandBoxGraphic(qreal width, qreal height, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_width(width), m_height(height) {
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


// ============================================================================
// 9. ThalesHeaderGraphic (Pasek stanu, logo THALES RSS, zegar - Arial)
// ============================================================================
ThalesHeaderGraphic::ThalesHeaderGraphic(QGraphicsItem* parent)
    : QGraphicsItem(parent) {
}

QRectF ThalesHeaderGraphic::boundingRect() const {
    return QRectF(0, 0, 340, 65);
}

void ThalesHeaderGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    // Kwadrat [P]
    painter->fillRect(QRectF(0, 0, 18, 18), QColor(0, 50, 220));
    QFont fontP("Arial", 9, QFont::Bold);
    painter->setFont(fontP);
    painter->setPen(Qt::white);
    painter->drawText(QRectF(0, 0, 18, 18), Qt::AlignCenter, "P");

    // Bloki [R] [G] [B]
    painter->fillRect(QRectF(24, 0, 18, 18), QColor(220, 0, 0));
    painter->drawText(QRectF(24, 0, 18, 18), Qt::AlignCenter, "R");

    painter->fillRect(QRectF(44, 0, 18, 18), QColor(0, 180, 0));
    painter->drawText(QRectF(44, 0, 18, 18), Qt::AlignCenter, "G");

    painter->fillRect(QRectF(64, 0, 18, 18), QColor(0, 50, 220));
    painter->drawText(QRectF(64, 0, 18, 18), Qt::AlignCenter, "B");

    // Zegar czasu rzeczywistego (odświeżany co sekundę)
    QDateTime now = QDateTime::currentDateTime();
    static const char* const days[] = {"PN", "WT", "ŚR", "CZ", "PT", "SO", "ND"};
    static const char* const months[] = {"STY", "LUT", "MAR", "KWI", "MAJ", "CZE", "LIP", "SIE", "WRZ", "PAŹ", "LIS", "GRU"};
    
    int dayOfWeek = now.date().dayOfWeek() - 1; // 1 = Monday -> 0
    if (dayOfWeek < 0 || dayOfWeek > 6) dayOfWeek = 0;
    int month = now.date().month() - 1;
    if (month < 0 || month > 11) month = 0;

    QString dateStr = QString("%1, %2-%3-%4")
        .arg(days[dayOfWeek])
        .arg(now.date().day(), 2, 10, QChar('0'))
        .arg(months[month])
        .arg(now.date().year());
    QString timeStr = now.time().toString("hh:mm:ss");

    QFont clockFont("Arial", 8, QFont::Normal);
    painter->setFont(clockFont);
    painter->setPen(QColor(180, 180, 180));
    painter->drawText(QRectF(90, 0, 140, 14), Qt::AlignRight, dateStr);
    painter->drawText(QRectF(90, 16, 140, 14), Qt::AlignRight, timeStr);

    // Paski kolorów nad logo
    painter->fillRect(QRectF(255, 2, 10, 3), QColor(0, 100, 255));
    painter->fillRect(QRectF(268, 2, 10, 3), Qt::white);
    painter->fillRect(QRectF(281, 2, 10, 3), QColor(255, 0, 0));

    // Logo THALES
    QFont logoFont("Arial", 10, QFont::Bold);
    painter->setFont(logoFont);
    painter->setPen(QColor(0, 80, 220));
    painter->drawText(QRectF(245, 12, 85, 16), Qt::AlignCenter, "THALES");

    // Logo RSS
    QFont rssFont("Arial", 11, QFont::Bold);
    painter->setFont(rssFont);
    painter->setPen(QColor(0, 255, 0));
    painter->drawText(QRectF(245, 28, 85, 18), Qt::AlignCenter, "RSS");
}


// ============================================================================
// 10. ThalesLabelGraphic
// ============================================================================
ThalesLabelGraphic::ThalesLabelGraphic(const QString& text, QColor color, bool isHeader, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_text(text), m_color(color), m_isHeader(isHeader) {
}

QRectF ThalesLabelGraphic::boundingRect() const {
    QFont font("Arial", m_isHeader ? 10 : 8, m_isHeader ? QFont::Bold : QFont::Normal);
    QFontMetrics fm(font);
    return QRectF(0, 0, fm.horizontalAdvance(m_text), fm.height());
}

void ThalesLabelGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    QFont font("Arial", m_isHeader ? 10 : 8, m_isHeader ? QFont::Bold : QFont::Normal);
    font.setStyleStrategy(QFont::PreferAntialias);
    painter->setFont(font);
    painter->setPen(m_color);

    painter->drawText(boundingRect(), Qt::AlignLeft | Qt::AlignTop, m_text);
}


// ============================================================================
// 11. ThalesLineBlockGraphic (Blokada liniowa z animacją zaznaczenia)
// ============================================================================
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


// ============================================================================
// 12. ThalesSignalBoxGraphic (Symbol Nastawni z animacją zaznaczenia)
// ============================================================================
ThalesSignalBoxGraphic::ThalesSignalBoxGraphic(const QString& label, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_label(label) {
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
    QGraphicsItem::mousePressEvent(event);
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



// ============================================================================
// 13. ThalesDerailGraphic (Wykolejnica)
// ============================================================================
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


// ============================================================================
// 14. ThalesAxleCounterGraphic (Licznik osi / Punkt pomiarowy)
// ============================================================================
ThalesAxleCounterGraphic::ThalesAxleCounterGraphic(const QString& sectionId, AxleState state, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_sectionId(sectionId), m_state(state) {
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF ThalesAxleCounterGraphic::boundingRect() const {
    return QRectF(-10, -18, 50, 30);
}

void ThalesAxleCounterGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    QGraphicsItem::mousePressEvent(event);
}

void ThalesAxleCounterGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    if (m_selected) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(160, 156, 0));
        painter->drawEllipse(QPointF(15, 0), 14.0, 10.0);
    }

    bool blinkOn = (QDateTime::currentMSecsSinceEpoch() / 500) % 2 == 0;

    QColor col;
    switch (m_state) {
        case OK:          col = QColor(155, 155, 155); break; // Szary
        case Occupied:    col = QColor(255, 0, 0);     break; // Czerwony
        case AxlePreReset:col = QColor(139, 0, 0);     break; // Ciemnoczerwony
        case Fault:       col = blinkOn ? QColor(255, 0, 0) : QColor(255, 255, 255); break; // Migający
    }

    // Tor z markerem punktu pomiarowego (dwa pionowe cienkie pasy jako granica sekcji)
    QPen trackPen(col, 4, Qt::SolidLine, Qt::FlatCap);
    painter->setPen(trackPen);
    painter->drawLine(QPointF(0, 0), QPointF(30, 0));

    // Pionowa linia separatora sekcji (marker licznika osi)
    QPen markerPen(QColor(220, 220, 220), 1, Qt::SolidLine, Qt::FlatCap);
    painter->setPen(markerPen);
    painter->drawLine(QPointF(15, -8), QPointF(15, 8));

    // Mały symbol diamentu nad torem (wizualny identyfikator licznika)
    QColor diamondCol = (m_state == Fault) ?
        (blinkOn ? QColor(255, 0, 0) : QColor(255, 255, 255)) : col;
    painter->setPen(Qt::NoPen);
    painter->setBrush(diamondCol);
    QPolygonF diamond;
    diamond << QPointF(15, -14) << QPointF(19, -9) << QPointF(15, -4) << QPointF(11, -9);
    painter->drawPolygon(diamond);

    // ID sekcji
    if (!m_sectionId.isEmpty()) {
        QFont font("Arial", 7, QFont::Normal);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        painter->setPen(QColor(180, 180, 180));
        painter->drawText(QRectF(-10, 6, 50, 12), Qt::AlignCenter, m_sectionId);
    }
}


// ============================================================================
// Główna Przeglądarka Komponentów Pulpitu Thales ML8
// ============================================================================
ThalesBrowserWindow::ThalesBrowserWindow(QWidget* parent)
    : QMainWindow(parent) {

    setWindowTitle("Thales ML8 / RSS - Przeglądarka Komponentów Pulpitu");
    resize(1080, 880);

    m_view = new QGraphicsView(this);
    m_scene = new QGraphicsScene(this);

    m_view->setScene(m_scene);
    m_view->setBackgroundBrush(QColor(33, 33, 33));
    m_view->setStyleSheet("border: none;");

    setCentralWidget(m_view);

    setupBrowser();

    // Scale całości o 40% względem domyślnego rozmiaru
    m_view->scale(1.4, 1.4);

    // Timer 500ms – odświeżanie zegara i elementów migających (FaultBlinking, Substitute, itp.)
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        m_scene->update();
    });
    timer->start(500);
}

void ThalesBrowserWindow::addSectionHeader(const QString& title, qreal x, qreal y) {
    auto header = new ThalesLabelGraphic(title, QColor(0, 200, 255), true);
    header->setPos(x, y);
    m_scene->addItem(header);

    QPen pen(QColor(60, 60, 60), 1);
    m_scene->addLine(x, y + 16, x + 980, y + 16, pen);
}

void ThalesBrowserWindow::setupBrowser() {
    // ------------------------------------------------------------------------
    // Sekcja 1: Przyciski funkcyjne
    // ------------------------------------------------------------------------
    addSectionHeader("1. PRZYCISKI FUNKCYJNE (WEKTOROWY KOD 3D, KLIKALNE)", 20, 20);

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

    auto lblBtnDesc = new ThalesLabelGraphic("Opis: Czysty wektorowy kod, ramka 3D, kliknij na dowolny obiekt!", QColor(130, 130, 130));
    lblBtnDesc->setPos(580, 52); m_scene->addItem(lblBtnDesc);


    // ------------------------------------------------------------------------
    // Sekcja 2: Odcinki torowe i kozły oporowe
    // ------------------------------------------------------------------------
    addSectionHeader("2. ODCINKI TOROWE (WSZYSTKIE 7 STANÓW + KOZŁY + NUMERY POCIĄGÓW)", 20, 100);

    int trkY = 135;
    auto trkFree = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free, ThalesTrackGraphic::None, "1");
    trkFree->setPos(30, trkY); m_scene->addItem(trkFree);
    auto lblTrk1 = new ThalesLabelGraphic("Wolny", QColor(155, 155, 155)); lblTrk1->setPos(30, trkY+13); m_scene->addItem(lblTrk1);

    auto trkOcc = new ThalesTrackGraphic(80, ThalesTrackGraphic::Occupied, ThalesTrackGraphic::None, "2");
    trkOcc->setPos(130, trkY); m_scene->addItem(trkOcc);
    auto lblTrk2 = new ThalesLabelGraphic("Zajęty", QColor(255, 0, 0)); lblTrk2->setPos(130, trkY+13); m_scene->addItem(lblTrk2);

    auto trkTrain = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedTrain, ThalesTrackGraphic::None, "3");
    trkTrain->setPos(230, trkY); m_scene->addItem(trkTrain);
    auto lblTrk3 = new ThalesLabelGraphic("Poc.", QColor(0, 255, 0)); lblTrk3->setPos(230, trkY+13); m_scene->addItem(lblTrk3);

    auto trkShunt = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedShunt, ThalesTrackGraphic::None, "4");
    trkShunt->setPos(330, trkY); m_scene->addItem(trkShunt);
    auto lblTrk4 = new ThalesLabelGraphic("Manewr.", QColor(255, 255, 0)); lblTrk4->setPos(330, trkY+13); m_scene->addItem(lblTrk4);

    auto trkMag = new ThalesTrackGraphic(80, ThalesTrackGraphic::MagentaRelease, ThalesTrackGraphic::None, "5");
    trkMag->setPos(430, trkY); m_scene->addItem(trkMag);
    auto lblTrk5m = new ThalesLabelGraphic("Zwaln.", QColor(255, 0, 255)); lblTrk5m->setPos(430, trkY+13); m_scene->addItem(lblTrk5m);

    auto trkFlt = new ThalesTrackGraphic(80, ThalesTrackGraphic::FaultBlinking, ThalesTrackGraphic::None, "6");
    trkFlt->setPos(530, trkY); m_scene->addItem(trkFlt);
    auto lblTrk6f = new ThalesLabelGraphic("Usterka", QColor(255, 80, 80)); lblTrk6f->setPos(530, trkY+13); m_scene->addItem(lblTrk6f);

    auto trkPre = new ThalesTrackGraphic(80, ThalesTrackGraphic::PreReset, ThalesTrackGraphic::None, "7");
    trkPre->setPos(630, trkY); m_scene->addItem(trkPre);
    auto lblTrk7p = new ThalesLabelGraphic("PreReset", QColor(139, 0, 0)); lblTrk7p->setPos(630, trkY+13); m_scene->addItem(lblTrk7p);
    
    auto trkTNumFree = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::None, "");
    trkTNumFree->setTrainNum("123456");
    trkTNumFree->setPos(730, trkY); m_scene->addItem(trkTNumFree);
    auto lblTNum1 = new ThalesLabelGraphic("Num.Wolny", QColor(155, 155, 155)); lblTNum1->setPos(730, trkY+13); m_scene->addItem(lblTNum1);

    auto trkTNumOcc = new ThalesTrackGraphic(100, ThalesTrackGraphic::Occupied, ThalesTrackGraphic::None, "");
    trkTNumOcc->setTrainNum("123456");
    trkTNumOcc->setPos(850, trkY); m_scene->addItem(trkTNumOcc);
    auto lblTNum2 = new ThalesLabelGraphic("Num.Zajęty", QColor(255, 0, 0)); lblTNum2->setPos(850, trkY+13); m_scene->addItem(lblTNum2);

    auto trkBufL = new ThalesTrackGraphic(40, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopLeft, "");
    trkBufL->setPos(980, trkY); m_scene->addItem(trkBufL);

    auto trkBufR = new ThalesTrackGraphic(40, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight, "");
    trkBufR->setPos(1040, trkY); m_scene->addItem(trkBufR);

    // ------------------------------------------------------------------------
    // Sekcja 3: Sygnalizatory w torze
    // ------------------------------------------------------------------------
    addSectionHeader("3. SYGNALIZATORY W TORZE (JEDNOLITY KOLOR SYGNAŁU)", 20, 185);

    int sigY = 225;
    
    auto trkSig1 = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free); trkSig1->setAcceptedMouseButtons(Qt::NoButton); trkSig1->setPos(30, sigY); m_scene->addItem(trkSig1);
    auto sig1 = new ThalesSignalGraphic("K1", ThalesSignalGraphic::TrainLeft, ThalesSignalGraphic::Stop, true); sig1->setPos(60, sigY); m_scene->addItem(sig1);
    auto lblSig1 = new ThalesLabelGraphic("Stój", QColor(155, 155, 155)); lblSig1->setPos(60, sigY+15); m_scene->addItem(lblSig1);

    auto trkSig2 = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedTrain); trkSig2->setAcceptedMouseButtons(Qt::NoButton); trkSig2->setPos(140, sigY); m_scene->addItem(trkSig2);
    auto sig2 = new ThalesSignalGraphic("H2", ThalesSignalGraphic::TrainRight, ThalesSignalGraphic::ProceedTrain, true); sig2->setPos(170, sigY); m_scene->addItem(sig2);
    auto lblSig2 = new ThalesLabelGraphic("Jazda P.", QColor(0, 255, 0)); lblSig2->setPos(170, sigY+15); m_scene->addItem(lblSig2);

    auto trkSig3 = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedShunt); trkSig3->setAcceptedMouseButtons(Qt::NoButton); trkSig3->setPos(250, sigY); m_scene->addItem(trkSig3);
    auto sig3 = new ThalesSignalGraphic("N1", ThalesSignalGraphic::ShuntLeft, ThalesSignalGraphic::ProceedShunt, true); sig3->setPos(280, sigY); m_scene->addItem(sig3);
    auto lblSig3 = new ThalesLabelGraphic("Manewr", QColor(255, 255, 0)); lblSig3->setPos(280, sigY+15); m_scene->addItem(lblSig3);

    auto trkSig4 = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free); trkSig4->setAcceptedMouseButtons(Qt::NoButton); trkSig4->setPos(360, sigY); m_scene->addItem(trkSig4);
    auto sig4 = new ThalesSignalGraphic("S1", ThalesSignalGraphic::TrainRight, ThalesSignalGraphic::SignalStopped, true); sig4->setPos(390, sigY); m_scene->addItem(sig4);
    auto lblSig4 = new ThalesLabelGraphic("Zatrzymany", QColor(255, 0, 255)); lblSig4->setPos(390, sigY+15); m_scene->addItem(lblSig4);

    auto trkSig5 = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free); trkSig5->setAcceptedMouseButtons(Qt::NoButton); trkSig5->setPos(470, sigY); m_scene->addItem(trkSig5);
    auto sig5 = new ThalesSignalGraphic("Z1", ThalesSignalGraphic::TrainLeft, ThalesSignalGraphic::Substitute, true); sig5->setPos(500, sigY); m_scene->addItem(sig5);
    auto lblSig5 = new ThalesLabelGraphic("Zastepczy(Miga)", QColor(255, 255, 255)); lblSig5->setPos(500, sigY+15); m_scene->addItem(lblSig5);

    // Semafor polsamoczynny z manewrowym - Stop
    auto trkSig6 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free); trkSig6->setAcceptedMouseButtons(Qt::NoButton); trkSig6->setPos(590, sigY); m_scene->addItem(trkSig6);
    auto sig6 = new ThalesSignalGraphic("H3", ThalesSignalGraphic::TrainAndShuntLeft, ThalesSignalGraphic::Stop, true); sig6->setPos(625, sigY); m_scene->addItem(sig6);
    auto lblSig6 = new ThalesLabelGraphic("Polsam+Man. Stoj", QColor(155, 155, 155)); lblSig6->setPos(625, sigY+15); m_scene->addItem(lblSig6);

    // Semafor polsamoczynny z manewrowym - Jazda pociagowa (zielony)
    auto trkSig7 = new ThalesTrackGraphic(100, ThalesTrackGraphic::RouteLockedTrain); trkSig7->setAcceptedMouseButtons(Qt::NoButton); trkSig7->setPos(730, sigY); m_scene->addItem(trkSig7);
    auto sig7 = new ThalesSignalGraphic("H4", ThalesSignalGraphic::TrainAndShuntRight, ThalesSignalGraphic::ProceedTrain, true); sig7->setPos(765, sigY); m_scene->addItem(sig7);
    auto lblSig7 = new ThalesLabelGraphic("Polsam+Man.Jazda", QColor(0, 255, 0)); lblSig7->setPos(765, sigY+15); m_scene->addItem(lblSig7);

    // Semafor polsamoczynny z manewrowym - Jazda manewrowa z pociagowym
    auto trkSig9 = new ThalesTrackGraphic(100, ThalesTrackGraphic::RouteLockedShunt); trkSig9->setAcceptedMouseButtons(Qt::NoButton); trkSig9->setPos(870, sigY); m_scene->addItem(trkSig9);
    auto sig9 = new ThalesSignalGraphic("Ms2", ThalesSignalGraphic::TrainAndShuntLeft, ThalesSignalGraphic::ProceedShunt, true); sig9->setPos(905, sigY); m_scene->addItem(sig9);
    auto lblSig9 = new ThalesLabelGraphic("Polsam Manewr+Poc.", QColor(255, 255, 0)); lblSig9->setPos(905, sigY+15); m_scene->addItem(lblSig9);


    // ------------------------------------------------------------------------
    // Sekcja 4: PKPM i Koniec Elektryfikacji
    // ------------------------------------------------------------------------
    addSectionHeader("4. PKPM (ŻÓŁTY TRÓJKĄT) ORAZ KONIEC SIECI TRAKCYJNEJ", 20, 285);
    
    int pkpmY = 325;
    auto trkPkpm1 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight); trkPkpm1->setAcceptedMouseButtons(Qt::NoButton); trkPkpm1->setPos(30, pkpmY); m_scene->addItem(trkPkpm1);
    auto pkpm108 = new ThalesPkpmGraphic(ThalesPkpmGraphic::Right, "108"); pkpm108->setPos(70, pkpmY); m_scene->addItem(pkpm108);

    auto trkPkpm2 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopLeft); trkPkpm2->setAcceptedMouseButtons(Qt::NoButton); trkPkpm2->setPos(150, pkpmY); m_scene->addItem(trkPkpm2);
    auto pkpm114 = new ThalesPkpmGraphic(ThalesPkpmGraphic::Left, "114"); pkpm114->setPos(180, pkpmY); m_scene->addItem(pkpm114);

    auto trkCat1 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight); trkCat1->setAcceptedMouseButtons(Qt::NoButton); trkCat1->setPos(300, pkpmY); m_scene->addItem(trkCat1);
    auto cat19 = new ThalesEndCatenaGraphic("19"); cat19->setPos(340, pkpmY); m_scene->addItem(cat19);
    auto pkpm19 = new ThalesPkpmGraphic(ThalesPkpmGraphic::Right); pkpm19->setPos(370, pkpmY); m_scene->addItem(pkpm19);

    auto trkCat2 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight); trkCat2->setAcceptedMouseButtons(Qt::NoButton); trkCat2->setPos(450, pkpmY); m_scene->addItem(trkCat2);
    auto cat128 = new ThalesEndCatenaGraphic("128"); cat128->setPos(490, pkpmY); m_scene->addItem(cat128);


    // ------------------------------------------------------------------------
    // Sekcja 5: Wykolejnice (Zgodne z wezel_poznanski.pdf)
    // ------------------------------------------------------------------------
    addSectionHeader("5. WYKOLEJNICE (WSZYSTKIE STANY)", 20, 385);
    int derY = 435;
    
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


    // ------------------------------------------------------------------------
    // Sekcja 6: Rozjazdy (Wszystkie 4 stany w ulozeniu na Wprost i na Bok)
    // ------------------------------------------------------------------------
    addSectionHeader("6. ROZJAZDY (WSZYSTKIE STANY ORAZ POLOZENIA)", 20, 485);
    int swY = 535;

    auto sw1 = new ThalesSwitchGraphic("SW1", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchNormal); sw1->setPos(50, swY); m_scene->addItem(sw1);
    auto lblSw1 = new ThalesLabelGraphic("Wprost (Wolna)", QColor(155,155,155)); lblSw1->setPos(50, swY+25); m_scene->addItem(lblSw1);

    auto sw2 = new ThalesSwitchGraphic("SW2", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchNormal); sw2->setDiverging(true); sw2->setPos(200, swY); m_scene->addItem(sw2);
    auto lblSw2 = new ThalesLabelGraphic("Bok (Wolna)", QColor(155,155,155)); lblSw2->setPos(200, swY+25); m_scene->addItem(lblSw2);

    auto sw3 = new ThalesSwitchGraphic("SW3", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchStopped); sw3->setPos(350, swY); m_scene->addItem(sw3);
    auto lblSw3 = new ThalesLabelGraphic("Zastopowana", QColor(255,0,255)); lblSw3->setPos(350, swY+25); m_scene->addItem(lblSw3);

    auto sw4 = new ThalesSwitchGraphic("SW4", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchNoControl); sw4->setPos(500, swY); m_scene->addItem(sw4);
    auto lblSw4 = new ThalesLabelGraphic("Brak Kontroli", QColor(255,255,255)); lblSw4->setPos(500, swY+25); m_scene->addItem(lblSw4);

    auto sw5 = new ThalesSwitchGraphic("SW5", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchDerailed); sw5->setPos(650, swY); m_scene->addItem(sw5);
    auto lblSw5 = new ThalesLabelGraphic("Rozpruta (Wprost)", QColor(255,0,0)); lblSw5->setPos(650, swY+25); m_scene->addItem(lblSw5);

    auto sw6 = new ThalesSwitchGraphic("SW6", ThalesSwitchGraphic::BranchUpRight, false, ThalesSwitchGraphic::SwitchDerailed); sw6->setDiverging(true); sw6->setPos(800, swY); m_scene->addItem(sw6);
    auto lblSw6 = new ThalesLabelGraphic("Rozpruta (Bok)", QColor(255,0,0)); lblSw6->setPos(800, swY+25); m_scene->addItem(lblSw6);

    // ------------------------------------------------------------------------
    // Sekcja 7: Terminal polecen i Wskazniki systemowe
    // ------------------------------------------------------------------------
    addSectionHeader("7. TERMINAL POLECEN ORAZ NAGLOWEK SYSTEMOWY", 20, 610);
    
    auto cmdBox = new ThalesCommandBoxGraphic(420, 75);
    cmdBox->setPos(30, 640);
    m_scene->addItem(cmdBox);

    m_headerGraphic = new ThalesHeaderGraphic();
    m_headerGraphic->setPos(500, 640);
    m_scene->addItem(m_headerGraphic);


    // ------------------------------------------------------------------------
    // Sekcja 8: Blokady Liniowe (Wszystkie 5 stanow) i Symbol Nastawni
    // ------------------------------------------------------------------------
    addSectionHeader("8. BLOKADY LINIOWE ORAZ SYMBOL NASTAWNI", 20, 750);
    int blkY = 790;

    auto blk1 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Neutral, "Neutral"); blk1->setPos(30, blkY); m_scene->addItem(blk1);
    auto blk2 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Sending, "Sending"); blk2->setPos(130, blkY); m_scene->addItem(blk2);
    auto blk3 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Receiving, "Receiving"); blk3->setPos(230, blkY); m_scene->addItem(blk3);
    auto blk4 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::PermissionRequested, "PermReq(Miga)"); blk4->setPos(330, blkY); m_scene->addItem(blk4);
    auto blk5 = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::EmergencyChange, "Emerg(Miga)"); blk5->setPos(430, blkY); m_scene->addItem(blk5);

    auto nast = new ThalesSignalBoxGraphic("Dz");
    nast->setPos(560, blkY-30);
    m_scene->addItem(nast);
    
    auto lblNast = new ThalesLabelGraphic("Ikona Nastawni", QColor(155, 155, 155));
    lblNast->setPos(560, blkY+10);
    m_scene->addItem(lblNast);

    m_scene->setSceneRect(0, 0, 1100, 1000);
}

QPointF ThalesSwitchGraphic::branchEndpoint() const {
    qreal endX = (m_dir == BranchUpRight || m_dir == BranchDownRight) ? 45.0 : 25.0;
    qreal endY = (m_dir == BranchUpRight || m_dir == BranchUpLeft) ? -25.0 : 25.0;
    return mapToScene(QPointF(endX, endY));
}
