#pragma once

#include "thales/graphics/thales_element_graphic.hpp"
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>

class ThalesTrackGraphic : public ThalesElementGraphic {
public:
    QString sectionId() const { return m_sectionId; }
    void setSectionId(const QString& id) { m_sectionId = id; update(); }
    void setSectionId(engine::core::UID uid) { m_uid = uid; update(); }
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

    ThalesTrackGraphic(qreal length, TrackState state = Free, Termination term = None, const QString& trackNum = QString(), QGraphicsItem* parent = nullptr, engine::core::UID uid = 0);
    ThalesTrackGraphic(qreal length, TrackState state, Termination term, const QString& trackNum, engine::core::UID uid, QGraphicsItem* parent = nullptr)
        : ThalesTrackGraphic(length, state, term, trackNum, parent, uid) {}
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
    bool m_selected{false};
};
