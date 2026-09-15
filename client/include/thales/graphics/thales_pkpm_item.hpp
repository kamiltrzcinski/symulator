#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QFont>
#include <QPainter>
#include <QDateTime>

class ThalesPkpmGraphic : public ThalesElementGraphic {
public:
    enum Direction {
        Left,   // Trójkąt w lewo ◀
        Right   // Trójkąt w prawo ▶
    };

    ThalesPkpmGraphic(Direction dir, const QString& trackNum = "", engine::core::UID uid, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Direction m_dir;
    QString m_trackNum;
    
};

