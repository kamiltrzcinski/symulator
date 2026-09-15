#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesSignalBoxGraphic : public ThalesElementGraphic {
public:
    QString name() const { return m_label; }
    void setName(const QString& n) { m_label = n; update(); }
    ThalesSignalBoxGraphic(const QString& label, QGraphicsItem* parent = nullptr, engine::core::UID uid = 0);
    ThalesSignalBoxGraphic(const QString& label, engine::core::UID uid, QGraphicsItem* parent = nullptr)
        : ThalesSignalBoxGraphic(label, parent, uid) {}
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_label;
    bool m_selected{false};
};
