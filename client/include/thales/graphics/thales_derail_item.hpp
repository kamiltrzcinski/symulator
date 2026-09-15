#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QFont>
#include <QPainter>
#include <QDateTime>

class ThalesDerailGraphic : public ThalesElementGraphic {
public:
    enum DerailState {
        Placed,     // Nałożona (zrzucająca) – gruba pionowa kreska na torze, czerwona
        Clear,      // Zdjęta (przejezdna) – dwie cienkie równoległe kreski
        Stopped,    // Zastopowanie wykolejnicy (magenta #FF00FF)
        NoControl   // Brak kontroli położenia (migający biały element)
    };

    enum Direction { Left, Right };

    ThalesDerailGraphic(const QString& name, Direction dir = Right, DerailState state = Clear, engine::core::UID uid, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    DerailState derailState() const { return m_state; }
    void setDerailState(DerailState s) { m_state = s; update(); }
    bool isOccupied() const { return m_occupied; }
    void setOccupied(bool o) { m_occupied = o; update(); }
    QString name() const { return m_name; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_name;
    Direction m_dir;
    DerailState m_state;
    bool m_occupied{false};
    
};

