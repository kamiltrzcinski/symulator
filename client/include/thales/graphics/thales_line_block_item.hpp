#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QFont>
#include <QPainter>
#include <QDateTime>

class ThalesLineBlockGraphic : public ThalesElementGraphic {
public:
    QString label() const { return m_label; }
    void setLabel(const QString& l) { m_label = l; update(); }
    enum State {
        Neutral,              // Brak nadanego kierunku (szare strzałki w obie strony)
        Sending,              // Wyjazd dozwolony / kierunek nadany (żółta strzałka w prawo)
        Receiving,            // Przyjazd dozwolony / kierunek odebrany (żółta strzałka w lewo)
        PermissionRequested,  // Żądanie pozwolenia (migająca żółta strzałka prążkowana)
        EmergencyChange       // Awaryjna zmiana kierunku (migająca czerwona)
    };
    ThalesLineBlockGraphic(State state = Neutral, const QString& label = "", engine::core::UID uid, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    State m_state;
    QString m_label;
    
};

