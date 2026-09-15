#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QFont>
#include <QPainter>
#include <QDateTime>

class ThalesLabelGraphic : public ThalesElementGraphic {
public:
    QString text() const { return m_text; }
    ThalesLabelGraphic(const QString& text, QColor color = QColor(155, 155, 155), bool isHeader = false, engine::core::UID uid, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setText(const QString& text) { m_text = text; update(); }
    void setPen(QColor color) { m_color = color; update(); }

private:
    QString m_text;
    QColor m_color;
    bool m_isHeader;
};

