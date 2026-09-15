#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesPkpmGraphic : public ThalesElementGraphic {
public:
    enum Direction {
        Left,   // Trójkąt w lewo ◀
        Right   // Trójkąt w prawo ▶
    };

    ThalesPkpmGraphic(Direction dir, const QString& trackNum = "", QGraphicsItem* parent = nullptr, engine::core::UID uid = {});
    ThalesPkpmGraphic(Direction dir, const QString& trackNum, engine::core::UID uid, QGraphicsItem* parent = nullptr)
        : ThalesPkpmGraphic(dir, trackNum, parent, uid) {}
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Direction m_dir;
    QString m_trackNum;
    bool m_selected{false};
};
