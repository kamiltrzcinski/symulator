#pragma once

#include <QString>
#include <QGraphicsScene>

class TopologyLoader {
public:
    static bool loadFromJson(const QString& filePath, QGraphicsScene* scene);
};
