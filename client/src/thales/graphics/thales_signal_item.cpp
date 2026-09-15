#include "thales/graphics/thales_signal_item.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QTimer>
#include <cmath>

// ============================================================================
ThalesSignalGraphic::ThalesSignalGraphic(const QString& name, SignalType type, SignalState state, bool labelAbove, QGraphicsItem* parent, engine::core::UID uid)
    : ThalesElementGraphic(parent, uid), m_name(name), m_type(type), m_state(state), m_labelAbove(labelAbove) {
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
    ThalesElementGraphic::mousePressEvent(event);
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


