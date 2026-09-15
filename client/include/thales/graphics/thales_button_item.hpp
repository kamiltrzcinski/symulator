#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QFont>
#include <QPainter>
#include <QDateTime>

class ThalesButtonGraphic : public ThalesElementGraphic {
public:
    engine::core::UID uid() const { return m_uid; }
    void setId(engine::core::UID uid) { m_uid = uid; }
    QString text() const { return m_text; }
    void setText(const QString& t) { m_text = t; update(); }
    enum Style {
        BlueOT,      // Wypełnienie granatowe, tekst jasnoniebieski (np. OTSKI1)
        GraySystem,  // Wypełnienie szare, tekst jasnoszary (np. LOFF, HMI)
        RedPzb       // Wypełnienie bordowe, tekst czerwony (np. PZB)
    };

    ThalesButtonGraphic(const QString& text, Style style = BlueOT, bool hasArrow = false, engine::core::UID uid, QGraphicsItem* parent = nullptr);
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
    
};

