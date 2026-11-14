#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesSignalGraphic : public ThalesElementGraphic {
public:
    enum SignalType {
        TrainLeft,          // Tylko pociągowy (semafor) w lewo <|
        TrainRight,         // Tylko pociągowy (semafor) w prawo |>
        ShuntLeft,          // Tylko manewrowy (tarcza) w lewo <
        ShuntRight,         // Tylko manewrowy (tarcza) w prawo >
        TrainAndShuntLeft,  // Pociągowy + manewrowy w lewo <<|
        TrainAndShuntRight  // Pociągowy + manewrowy w prawo |>>
    };

    enum SignalState {
        Stop,               // Sygnał Stój (kolor szary toru #9B9B9B)
        ProceedTrain,       // Przebieg pociągowy (Zielony #00FF00)
        ProceedShunt,       // Przebieg manewrowy (Żółty #FFFF00)
        Substitute,         // Sygnał zastępczy Sz (migający biały – priorytet nad Stój)
        SignalStopped       // Zastopowanie sygnalizatora (magenta #FF00FF)
    };

    QString name() const { return m_name; }
    void setName(const QString& n) { m_name = n; update(); }
    SignalType signalType() const { return m_type; }
    void setSignalType(SignalType t) { m_type = t; prepareGeometryChange(); update(); }

    ThalesSignalGraphic(const QString& name, SignalType type, SignalState state = Stop, bool labelAbove = true, QGraphicsItem* parent = nullptr, engine::core::UID uid = {});
    ThalesSignalGraphic(const QString& name, SignalType type, SignalState state, bool labelAbove, engine::core::UID uid, QGraphicsItem* parent = nullptr)
        : ThalesSignalGraphic(name, type, state, labelAbove, parent, uid) {}
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QString m_name;
    SignalType m_type;
    SignalState m_state;
    bool m_labelAbove;
    bool m_selected{false};
};
