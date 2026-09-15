#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesCommandBoxGraphic : public ThalesElementGraphic {
public:
    ThalesCommandBoxGraphic(qreal width = 450, qreal height = 70, QGraphicsItem* parent = nullptr, engine::core::UID uid = 0);
    ThalesCommandBoxGraphic(qreal width, qreal height, engine::core::UID uid, QGraphicsItem* parent = nullptr)
        : ThalesCommandBoxGraphic(width, height, parent, uid) {}
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setWeText(const QString& text) { m_weText = text; update(); }
    void setKomText(const QString& text) { m_komText = text; update(); }
    void setXxText(const QString& text) { m_xxText = text; update(); }

private:
    qreal m_width;
    qreal m_height;
    QString m_weText;
    QString m_komText;
    QString m_xxText;
};
