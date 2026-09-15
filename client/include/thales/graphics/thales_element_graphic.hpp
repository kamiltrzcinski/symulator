#pragma once

#include <engine/core/types.hpp>
#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

class ThalesElementGraphic : public QGraphicsObject {
    Q_OBJECT
public:
    explicit ThalesElementGraphic(QGraphicsItem* parent = nullptr, engine::core::UID uid = 0);
    ~ThalesElementGraphic() override = default;

    engine::core::UID uid() const { return m_uid; }
    void setUid(engine::core::UID uid) { m_uid = uid; }

    bool isSelectedState() const { return m_selected; }
    void setSelectedState(bool selected);

signals:
    void clicked(engine::core::UID uid);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    engine::core::UID m_uid{0};
    bool m_selected{false};
};
