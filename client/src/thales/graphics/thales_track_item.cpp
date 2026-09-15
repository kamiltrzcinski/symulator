#include "thales/graphics/thales_track_item.hpp"
#include <QGraphicsSceneMouseEvent>

ThalesTrackGraphic::ThalesTrackGraphic(qreal length, TrackState state, Termination term, const QString& trackNum, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_length(length), m_state(state), m_termination(term), m_trackNum(trackNum) {
    setAcceptHoverEvents(true);
}

QRectF ThalesTrackGraphic::boundingRect() const {
    return QRectF(-6, -24, m_length + 12, 40);
}

void ThalesTrackGraphic::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selected = !m_selected;
        update();
    }
    QGraphicsItem::mousePressEvent(event);
}

void ThalesTrackGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, false);

    // 1. Wyraźny żółty pas zaznaczenia odcinka torowego (#A09C00 / RGB(160, 156, 0))
    // Wysokość 12px (od y=-6 do y=6), szerokość ~40px lub 70% długości toru, rysowany w tle
    if (m_selected) {
        qreal selW = qMin(42.0, m_length * 0.7);
        qreal selX = (m_length - selW) / 2.0;
        painter->fillRect(QRectF(selX, -6.0, selW, 12.0), QColor(160, 156, 0));
    }

    QColor col;
    switch (m_state) {
        case Free:             col = QColor(155, 155, 155); break; // Szary #9B9B9B
        case Occupied:         col = QColor(255, 0, 0);     break; // Czerwony #FF0000
        case RouteLockedTrain: col = QColor(0, 255, 0);     break; // Zieleń (#00FF00)
        case RouteLockedShunt: col = QColor(255, 255, 0);   break; // Żółty
        case MagentaRelease:   col = QColor(255, 0, 255);   break; // Magenta #FF00FF
        case PreReset:         col = QColor(139, 0, 0);     break; // Ciemnoczerwony #8B0000
        case FaultBlinking: {
            // Czerwono-biały migający: zmiana co 500ms
            bool blinkOn = (QDateTime::currentMSecsSinceEpoch() / 500) % 2 == 0;
            col = blinkOn ? QColor(255, 0, 0) : QColor(255, 255, 255);
            break;
        }
    }

    // 2. Rysowanie toru zasadniczego (grubość 4px: od y=-2 do y=2)
    QPen trackPen(col, 4, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
    painter->setPen(trackPen);

    if (m_state == FaultBlinking) {
        // Cały odcinek miga: najpierw czerwony, potem biały (nie prążki!)
        bool blinkOn = (QDateTime::currentMSecsSinceEpoch() / 500) % 2 == 0;
        QColor flashCol = blinkOn ? QColor(255, 0, 0) : QColor(255, 255, 255);
        QPen flashPen(flashCol, 4, Qt::SolidLine, Qt::FlatCap);
        painter->setPen(flashPen);
        if (!m_trainNum.isEmpty()) {
            QFontMetrics fm(QFont("Consolas", 10, QFont::Normal));
            int tw = fm.horizontalAdvance(m_trainNum);
            qreal gapStart = (m_length - tw) / 2.0 - 4;
            qreal gapEnd = gapStart + tw + 8;
            painter->drawLine(QPointF(0, 0), QPointF(gapStart, 0));
            painter->drawLine(QPointF(gapEnd, 0), QPointF(m_length, 0));
        } else {
            painter->drawLine(QPointF(0, 0), QPointF(m_length, 0));
        }
    } else {
        if (!m_trainNum.isEmpty()) {
            QFontMetrics fm(QFont("Consolas", 10, QFont::Normal));
            int tw = fm.horizontalAdvance(m_trainNum);
            qreal gapStart = (m_length - tw) / 2.0 - 4;
            qreal gapEnd = gapStart + tw + 8;
            painter->drawLine(QPointF(0, 0), QPointF(gapStart, 0));
            painter->drawLine(QPointF(gapEnd, 0), QPointF(m_length, 0));
        } else {
            painter->drawLine(QPointF(0, 0), QPointF(m_length, 0));
        }
    }

    // Kozły oporowe: czysty prostokątny blok 4px x 12px
    if (m_termination == BufferStopLeft) {
        painter->fillRect(QRectF(0, -6, 4, 12), col);
    } else if (m_termination == BufferStopRight) {
        painter->fillRect(QRectF(m_length - 4, -6, 4, 12), col);
    }

    // 3. Numer toru umieszczony WYRAŹNIE NAD torem (y=-18 do y=-6, nie nachodzi na szynę)
    if (!m_trackNum.isEmpty()) {
        QFont font("Arial", 8, QFont::Normal);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        painter->setPen(QColor(180, 180, 180));
        painter->drawText(QRectF(0, -19, m_length, 14), Qt::AlignCenter, m_trackNum);
    }
    
    // Numer pociągu przecinający tor
    if (!m_trainNum.isEmpty()) {
        QFont font("Consolas", 10, QFont::Normal);
        font.setStyleStrategy(QFont::PreferAntialias);
        painter->setFont(font);
        
        // Draw text without background because we left a physical gap in the line
        painter->setPen(QColor(0, 255, 0)); // Green text
        painter->drawText(QRectF(0, -10, m_length, 20), Qt::AlignCenter, m_trainNum);
    }
}

    auto trkFree = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free, ThalesTrackGraphic::None, "1");
    trkFree->setPos(30, trkY); m_scene->addItem(trkFree);

    auto trkOcc = new ThalesTrackGraphic(80, ThalesTrackGraphic::Occupied, ThalesTrackGraphic::None, "2");
    trkOcc->setPos(130, trkY); m_scene->addItem(trkOcc);

    auto trkTrain = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedTrain, ThalesTrackGraphic::None, "3");
    trkTrain->setPos(230, trkY); m_scene->addItem(trkTrain);

    auto trkShunt = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedShunt, ThalesTrackGraphic::None, "4");
    trkShunt->setPos(330, trkY); m_scene->addItem(trkShunt);

    auto trkMag = new ThalesTrackGraphic(80, ThalesTrackGraphic::MagentaRelease, ThalesTrackGraphic::None, "5");
    trkMag->setPos(430, trkY); m_scene->addItem(trkMag);

    auto trkFlt = new ThalesTrackGraphic(80, ThalesTrackGraphic::FaultBlinking, ThalesTrackGraphic::None, "6");
    trkFlt->setPos(530, trkY); m_scene->addItem(trkFlt);

    auto trkPre = new ThalesTrackGraphic(80, ThalesTrackGraphic::PreReset, ThalesTrackGraphic::None, "7");
    trkPre->setPos(630, trkY); m_scene->addItem(trkPre);

    auto trkTNumFree = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::None, "");
    trkTNumFree->setTrainNum("123456");

    auto trkTNumOcc = new ThalesTrackGraphic(100, ThalesTrackGraphic::Occupied, ThalesTrackGraphic::None, "");
    trkTNumOcc->setTrainNum("123456");

    auto trkBufL = new ThalesTrackGraphic(40, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopLeft, "");
    trkBufL->setPos(980, trkY); m_scene->addItem(trkBufL);

    auto trkBufR = new ThalesTrackGraphic(40, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight, "");
    trkBufR->setPos(1040, trkY); m_scene->addItem(trkBufR);

    auto trkSig1 = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free); trkSig1->setAcceptedMouseButtons(Qt::NoButton); trkSig1->setPos(30, sigY); m_scene->addItem(trkSig1);
    auto sig1 = new ThalesSignalGraphic("K1", ThalesSignalGraphic::TrainLeft, ThalesSignalGraphic::Stop, true); sig1->setPos(60, sigY); m_scene->addItem(sig1);

    auto trkSig2 = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedTrain); trkSig2->setAcceptedMouseButtons(Qt::NoButton); trkSig2->setPos(140, sigY); m_scene->addItem(trkSig2);
    auto sig2 = new ThalesSignalGraphic("H2", ThalesSignalGraphic::TrainRight, ThalesSignalGraphic::ProceedTrain, true); sig2->setPos(170, sigY); m_scene->addItem(sig2);

    auto trkSig3 = new ThalesTrackGraphic(80, ThalesTrackGraphic::RouteLockedShunt); trkSig3->setAcceptedMouseButtons(Qt::NoButton); trkSig3->setPos(250, sigY); m_scene->addItem(trkSig3);
    auto sig3 = new ThalesSignalGraphic("N1", ThalesSignalGraphic::ShuntLeft, ThalesSignalGraphic::ProceedShunt, true); sig3->setPos(280, sigY); m_scene->addItem(sig3);

    auto trkSig4 = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free); trkSig4->setAcceptedMouseButtons(Qt::NoButton); trkSig4->setPos(360, sigY); m_scene->addItem(trkSig4);
    auto sig4 = new ThalesSignalGraphic("S1", ThalesSignalGraphic::TrainRight, ThalesSignalGraphic::SignalStopped, true); sig4->setPos(390, sigY); m_scene->addItem(sig4);

    auto trkSig5 = new ThalesTrackGraphic(80, ThalesTrackGraphic::Free); trkSig5->setAcceptedMouseButtons(Qt::NoButton); trkSig5->setPos(470, sigY); m_scene->addItem(trkSig5);
    auto sig5 = new ThalesSignalGraphic("Z1", ThalesSignalGraphic::TrainLeft, ThalesSignalGraphic::Substitute, true); sig5->setPos(500, sigY); m_scene->addItem(sig5);

    auto trkSig6 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free); trkSig6->setAcceptedMouseButtons(Qt::NoButton); trkSig6->setPos(590, sigY); m_scene->addItem(trkSig6);
    auto sig6 = new ThalesSignalGraphic("H3", ThalesSignalGraphic::TrainAndShuntLeft, ThalesSignalGraphic::Stop, true); sig6->setPos(625, sigY); m_scene->addItem(sig6);

    auto trkSig7 = new ThalesTrackGraphic(100, ThalesTrackGraphic::RouteLockedTrain); trkSig7->setAcceptedMouseButtons(Qt::NoButton); trkSig7->setPos(730, sigY); m_scene->addItem(trkSig7);
    auto sig7 = new ThalesSignalGraphic("H4", ThalesSignalGraphic::TrainAndShuntRight, ThalesSignalGraphic::ProceedTrain, true); sig7->setPos(765, sigY); m_scene->addItem(sig7);

    auto trkSig9 = new ThalesTrackGraphic(100, ThalesTrackGraphic::RouteLockedShunt); trkSig9->setAcceptedMouseButtons(Qt::NoButton); trkSig9->setPos(870, sigY); m_scene->addItem(trkSig9);
    auto sig9 = new ThalesSignalGraphic("Ms2", ThalesSignalGraphic::TrainAndShuntLeft, ThalesSignalGraphic::ProceedShunt, true); sig9->setPos(905, sigY); m_scene->addItem(sig9);

    auto trkPkpm1 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight); trkPkpm1->setAcceptedMouseButtons(Qt::NoButton); trkPkpm1->setPos(30, pkpmY); m_scene->addItem(trkPkpm1);
    auto pkpm108 = new ThalesPkpmGraphic(ThalesPkpmGraphic::Right, "108"); pkpm108->setPos(70, pkpmY); m_scene->addItem(pkpm108);

    auto trkPkpm2 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopLeft); trkPkpm2->setAcceptedMouseButtons(Qt::NoButton); trkPkpm2->setPos(150, pkpmY); m_scene->addItem(trkPkpm2);
    auto pkpm114 = new ThalesPkpmGraphic(ThalesPkpmGraphic::Left, "114"); pkpm114->setPos(180, pkpmY); m_scene->addItem(pkpm114);

    auto trkCat1 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight); trkCat1->setAcceptedMouseButtons(Qt::NoButton); trkCat1->setPos(300, pkpmY); m_scene->addItem(trkCat1);
    auto cat19 = new ThalesEndCatenaGraphic("19"); cat19->setPos(340, pkpmY); m_scene->addItem(cat19);

    auto trkCat2 = new ThalesTrackGraphic(100, ThalesTrackGraphic::Free, ThalesTrackGraphic::BufferStopRight); trkCat2->setAcceptedMouseButtons(Qt::NoButton); trkCat2->setPos(450, pkpmY); m_scene->addItem(trkCat2);
    auto cat128 = new ThalesEndCatenaGraphic("128"); cat128->setPos(490, pkpmY); m_scene->addItem(cat128);
