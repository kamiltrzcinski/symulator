#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesPlatformGraphic : public ThalesElementGraphic {
public:
    ThalesPlatformGraphic(const QString& text, qreal width = 85, qreal height = 20, QGraphicsItem* parent = nullptr, engine::core::UID uid = {});
    ThalesPlatformGraphic(const QString& text, qreal width, qreal height, engine::core::UID uid, QGraphicsItem* parent = nullptr)
        : ThalesPlatformGraphic(text, width, height, parent, uid) {}
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QString m_text;
    qreal m_width;
    qreal m_height;
};
