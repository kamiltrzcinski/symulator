#include "thales/graphics/thales_switch_item.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QTimer>
#include <cmath>

// ============================================================================
ThalesSwitchGraphic::ThalesSwitchGraphic(const QString& name, BranchDir dir, bool divergingOccupied, SwitchState state, QGraphicsItem* parent, engine::core::UID uid)
    : ThalesElementGraphic(parent, uid), m_name(name), m_dir(dir), m_divergingOccupied(divergingOccupied), m_switchState(state) {
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
    ThalesElementGraphic::mousePressEvent(event);
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


QPointF ThalesSwitchGraphic::branchEndpoint() const {
    qreal endX = (m_dir == BranchUpRight || m_dir == BranchDownRight) ? 45.0 : 25.0;
    qreal endY = (m_dir == BranchUpRight || m_dir == BranchUpLeft) ? -25.0 : 25.0;
    return mapToScene(QPointF(endX, endY));
}
