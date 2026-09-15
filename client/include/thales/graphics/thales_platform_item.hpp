#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QFont>
#include <QPainter>
#include <QDateTime>

class ThalesPlatformGraphic : public ThalesElementGraphic {
public:
    ThalesPlatformGraphic(const QString& text, qreal width = 85, qreal height = 20, engine::core::UID uid, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QString m_text;
    qreal m_width;
    qreal m_height;
};

