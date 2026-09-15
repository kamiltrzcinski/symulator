#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesSwitchGraphic : public ThalesElementGraphic {
public:
    enum BranchDir {
        BranchUpRight,
        BranchDownRight,
        BranchUpLeft,
        BranchDownLeft
    };

    enum SwitchState {
        SwitchNormal,    // Normalny (szary wg zajętości)
        SwitchStopped,   // Zastopowanie / zamknięcie indywidualne (magenta #FF00FF)
        SwitchNoControl, // Brak kontroli położenia (migający biały kwadrat u podstawy)
        SwitchDerailed   // Rozprucie zwrotnicy (migający czerwony)
    };
    
    QString name() const { return m_name; }
    void setName(const QString& n) { m_name = n; update(); }
    BranchDir branch() const { return m_dir; }
    void setBranch(BranchDir d) { m_dir = d; prepareGeometryChange(); update(); }
    SwitchState switchState() const { return m_switchState; }
    void setSwitchState(SwitchState s) { m_switchState = s; update(); }
    bool isDiverging() const { return m_diverging; }
    void setDiverging(bool d) { m_diverging = d; update(); }
    QPointF branchEndpoint() const;

    ThalesSwitchGraphic(const QString& name, BranchDir dir, bool divergingOccupied = false, SwitchState state = SwitchNormal, QGraphicsItem* parent = nullptr, engine::core::UID uid = {});
    ThalesSwitchGraphic(engine::core::UID uid, BranchDir dir, bool divergingOccupied = false, SwitchState state = SwitchNormal, QGraphicsItem* parent = nullptr)
        : ThalesSwitchGraphic(QString::number(uid.value), dir, divergingOccupied, state, parent, uid) {}
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_name;
    BranchDir m_dir;
    bool m_divergingOccupied;
    SwitchState m_switchState;
    bool m_diverging{false};
    bool m_selected{false};
};
