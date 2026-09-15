#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesButtonGraphic : public ThalesElementGraphic {
public:
    QString id() const { return m_id; }
    void setId(const QString& id) { m_id = id; }
    QString text() const { return m_text; }
    void setText(const QString& t) { m_text = t; update(); }
    enum Style {
        BlueOT,      // Wypełnienie granatowe, tekst jasnoniebieski (np. OTSKI1)
        GraySystem,  // Wypełnienie szare, tekst jasnoszary (np. LOFF, HMI)
        RedPzb       // Wypełnienie bordowe, tekst czerwony (np. PZB)
    };

    ThalesButtonGraphic(const QString& text, Style style = BlueOT, bool hasArrow = false, QGraphicsItem* parent = nullptr, engine::core::UID uid = {});
    ThalesButtonGraphic(const QString& text, Style style, bool hasArrow, engine::core::UID uid, QGraphicsItem* parent = nullptr)
        : ThalesButtonGraphic(text, style, hasArrow, parent, uid) {}
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_text;
    Style m_style;
    bool m_hasArrow;
    bool m_pressed{false};
    QString m_id;
};
