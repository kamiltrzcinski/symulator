#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesHeaderGraphic : public ThalesElementGraphic {
public:
    ThalesHeaderGraphic(QGraphicsItem* parent = nullptr, engine::core::UID uid = {});
    ThalesHeaderGraphic(engine::core::UID uid, QGraphicsItem* parent = nullptr)
        : ThalesHeaderGraphic(parent, uid) {}
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
};
