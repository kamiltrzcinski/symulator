#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesEndCatenaGraphic : public ThalesElementGraphic {
public:
    ThalesEndCatenaGraphic(const QString& trackNum = "", QGraphicsItem* parent = nullptr, engine::core::UID uid = {});
    ThalesEndCatenaGraphic(const QString& trackNum, engine::core::UID uid, QGraphicsItem* parent = nullptr)
        : ThalesEndCatenaGraphic(trackNum, parent, uid) {}
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_trackNum;
    bool m_selected{false};
};
