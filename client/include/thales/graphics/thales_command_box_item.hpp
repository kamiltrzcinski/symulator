#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QFont>
#include <QPainter>
#include <QDateTime>

class ThalesCommandBoxGraphic : public ThalesElementGraphic {
public:
    ThalesCommandBoxGraphic(qreal width = 450, qreal height = 70, engine::core::UID uid, QGraphicsItem* parent = nullptr);
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

