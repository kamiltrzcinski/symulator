#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QFont>
#include <QPainter>
#include <QDateTime>

class ThalesSignalBoxGraphic : public ThalesElementGraphic {
public:
    QString name() const { return m_label; }
    void setName(const QString& n) { m_label = n; update(); }
    ThalesSignalBoxGraphic(const QString& label, engine::core::UID uid, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_label;
    
};

