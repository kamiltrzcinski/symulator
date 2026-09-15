#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QFont>
#include <QPainter>
#include <QDateTime>

class ThalesTrackGraphic : public ThalesElementGraphic {
public:
    QString sectionId() const { return m_sectionId; }
    void setSectionId(engine::core::UID uid) { m_sectionId = id; update(); }
    qreal length() const { return m_length; }
    void setLength(qreal len) { m_length = len; prepareGeometryChange(); update(); }
    enum TrackState {
        Free,               // Wolny (jasnoszary #9B9B9B, 4px)
        Occupied,           // Zajęty (czerwony #FF0000, 4px)
        RouteLockedTrain,   // Utwierdzony pociągowy (zielony #00FF00, 4px)
        RouteLockedShunt,   // Utwierdzony manewrowy (żółty #FFFF00, 4px)
        MagentaRelease,     // Zwalnianie z opóźnieniem (magenta #FF00FF, 4px)
        FaultBlinking,      // Usterka licznika osi (czerwono-biały migający)
        PreReset            // Reset wstępny / oczekiwanie na pociąg (ciemnoczerwony #8B0000)
    };

    enum Termination {
        None,
        BufferStopLeft,     // Kozioł oporowy po lewej
        BufferStopRight     // Kozioł oporowy po prawej
    };

    ThalesTrackGraphic(qreal length, TrackState state = Free, Termination term = None, const QString& trackNum = QString(), engine::core::UID uid, QGraphicsItem* parent = nullptr);
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void setTrainNum(const QString& num) { m_trainNum = num; update(); }
    QString trainNum() const { return m_trainNum; }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    qreal m_length;
    QString m_sectionId;
    TrackState m_state;
    Termination m_termination;
    QString m_trackNum;
    QString m_trainNum;
    
};

