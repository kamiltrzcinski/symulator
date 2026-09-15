#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesLabelGraphic : public ThalesElementGraphic {
public:
    QString text() const { return m_text; }
    ThalesLabelGraphic(const QString& text, QColor color = QColor(155, 155, 155), bool isHeader = false, QGraphicsItem* parent = nullptr, engine::core::UID uid = 0);
    ThalesLabelGraphic(const QString& text, QColor color, bool isHeader, engine::core::UID uid, QGraphicsItem* parent = nullptr)
        : ThalesLabelGraphic(text, color, isHeader, parent, uid) {}
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setText(const QString& text) { m_text = text; update(); }
    void setPen(QColor color) { m_color = color; update(); }

private:
    QString m_text;
    QColor m_color;
    bool m_isHeader;
};
