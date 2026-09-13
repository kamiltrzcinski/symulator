#include "thales/topology_loader.hpp"
#include "thales/thales_browser.hpp"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

bool TopologyLoader::loadFromJson(const QString& filePath, QGraphicsScene* scene) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    if (doc.isNull()) {
        qWarning() << "Invalid JSON in" << filePath;
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray elements = root["elements"].toArray();

    for (int i = 0; i < elements.size(); ++i) {
        QJsonObject el = elements[i].toObject();
        QString type = el["type"].toString();
        qreal x = el["x"].toDouble();
        qreal y = el["y"].toDouble();
        qreal rot = el["rot"].toDouble();

        QGraphicsItem* item = nullptr;

        if (type == "CommandBox") {
            item = new ThalesCommandBoxGraphic(544, 52);
        } else if (type == "PZB") {
            auto pzb = new QGraphicsRectItem(0, 0, 25, 12);
            pzb->setBrush(QColor(139, 0, 0)); pzb->setPen(Qt::NoPen);
            auto text = new QGraphicsTextItem("PZB", pzb);
            text->setDefaultTextColor(Qt::white);
            text->setFont(QFont("Arial", 6, QFont::Bold));
            text->setPos(0, -2);
            item = pzb;
        } else if (type == "LeftPRGB") {
            auto group = new QGraphicsItemGroup();
            auto pRect = new QGraphicsRectItem(0, 0, 18, 18, group);
            pRect->setPen(QPen(QColor(0, 50, 220), 2));
            auto pText = new QGraphicsTextItem("P", pRect);
            pText->setDefaultTextColor(QColor(0, 50, 220)); pText->setFont(QFont("Arial", 9, QFont::Bold)); pText->setPos(0, -1);
            
            auto rRect = new QGraphicsRectItem(25, 0, 18, 18, group);
            rRect->setBrush(QColor(220, 0, 0)); rRect->setPen(Qt::NoPen);
            auto rText = new QGraphicsTextItem("R", rRect);
            rText->setDefaultTextColor(Qt::white); rText->setFont(QFont("Arial", 9, QFont::Bold)); rText->setPos(25, -1);
            
            auto gRect = new QGraphicsRectItem(45, 0, 18, 18, group);
            gRect->setBrush(QColor(0, 180, 0)); gRect->setPen(Qt::NoPen);
            auto gText = new QGraphicsTextItem("G", gRect);
            gText->setDefaultTextColor(Qt::black); gText->setFont(QFont("Arial", 9, QFont::Bold)); gText->setPos(45, -1);
            
            auto bRect = new QGraphicsRectItem(65, 0, 18, 18, group);
            bRect->setBrush(QColor(0, 50, 220)); bRect->setPen(Qt::NoPen);
            auto bText = new QGraphicsTextItem("B", bRect);
            bText->setDefaultTextColor(Qt::white); bText->setFont(QFont("Arial", 9, QFont::Bold)); bText->setPos(65, -1);
            item = group;
        } else if (type == "Header") {
            item = new ThalesHeaderGraphic();
        } else if (type == "Label") {
            item = new ThalesLabelGraphic(el["text"].toString(), QColor(el["color"].toString()));
        } else if (type == "Button") {
            item = new ThalesButtonGraphic(el["text"].toString(), ThalesButtonGraphic::GraySystem);
        } else if (type == "LineBlock") {
            item = new ThalesLineBlockGraphic(ThalesLineBlockGraphic::Neutral, el["id"].toString());
        } else if (type == "Signal") {
            QString sigType = el["sigType"].toString();
            ThalesSignalGraphic::SignalType st = ThalesSignalGraphic::TrainRight;
            if (sigType == "TrainRight") st = ThalesSignalGraphic::TrainRight;
            else if (sigType == "TrainLeft") st = ThalesSignalGraphic::TrainLeft;
            else if (sigType == "ShuntRight") st = ThalesSignalGraphic::ShuntRight;
            else if (sigType == "ShuntLeft") st = ThalesSignalGraphic::ShuntLeft;
            else if (sigType == "TrainAndShuntLeft") st = ThalesSignalGraphic::TrainAndShuntLeft;
            else if (sigType == "TrainAndShuntRight") st = ThalesSignalGraphic::TrainAndShuntRight;
            item = new ThalesSignalGraphic(el["id"].toString(), st, ThalesSignalGraphic::Stop, el["labelAbove"].toBool(true));
        } else if (type == "Track") {
            item = new ThalesTrackGraphic(el["length"].toDouble(), ThalesTrackGraphic::Free);
        } else if (type == "Switch") {
            QString b = el["branch"].toString();
            ThalesSwitchGraphic::BranchDir dir = ThalesSwitchGraphic::BranchDownRight;
            if (b == "BranchUpRight") dir = ThalesSwitchGraphic::BranchUpRight;
            
            
            item = new ThalesSwitchGraphic(el["id"].toString(), dir);
        } else if (type == "Platform") {
            item = new ThalesPlatformGraphic(el["text"].toString(), el["w"].toDouble(), el["h"].toDouble());
        } else if (type == "Pkpm") {
            ThalesPkpmGraphic::Direction d = (el["dir"].toString() == "Left") ? ThalesPkpmGraphic::Left : ThalesPkpmGraphic::Right;
            item = new ThalesPkpmGraphic(d);
        }

        if (item) {
            item->setPos(x, y);
            item->setRotation(rot);
            scene->addItem(item);
        }
    }
    return true;
}
