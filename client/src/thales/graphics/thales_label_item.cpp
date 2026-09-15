#include "thales/graphics/thales_label_item.hpp"
#include <QGraphicsSceneMouseEvent>

ThalesLabelGraphic::ThalesLabelGraphic(const QString& text, QColor color, bool isHeader, QGraphicsItem* parent)
    : QGraphicsItem(parent), m_text(text), m_color(color), m_isHeader(isHeader) {
}

QRectF ThalesLabelGraphic::boundingRect() const {
    QFont font("Arial", m_isHeader ? 10 : 8, m_isHeader ? QFont::Bold : QFont::Normal);
    QFontMetrics fm(font);
    return QRectF(0, 0, fm.horizontalAdvance(m_text), fm.height());
}

void ThalesLabelGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    QFont font("Arial", m_isHeader ? 10 : 8, m_isHeader ? QFont::Bold : QFont::Normal);
    font.setStyleStrategy(QFont::PreferAntialias);
    painter->setFont(font);
    painter->setPen(m_color);

    painter->drawText(boundingRect(), Qt::AlignLeft | Qt::AlignTop, m_text);
}
