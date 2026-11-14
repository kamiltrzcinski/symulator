#include "thales/graphics/thales_platform_item.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QTimer>
#include <cmath>

// Perony NIE podlegają zaznaczaniu myszą.
// ============================================================================
ThalesPlatformGraphic::ThalesPlatformGraphic(const QString& text, qreal width, qreal height, QGraphicsItem* parent, engine::core::UID uid)
    : ThalesElementGraphic(parent, uid), m_text(text), m_width(width), m_height(height) {
}

QRectF ThalesPlatformGraphic::boundingRect() const {
    return QRectF(0, 0, m_width, m_height);
}

void ThalesPlatformGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    QRectF r(0, 0, m_width, m_height);

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    // Czyste tło pulpitu wewnątrz peronu
    painter->fillRect(r, QColor(33, 33, 33));

    QPen borderPen(QColor(155, 155, 155), 1);
    painter->setPen(borderPen);

    // Podwójna linia na górze peronu (odstęp 1px, czyli linie obok siebie z jednym pikselem przerwy)
    painter->drawLine(QPointF(0, 0), QPointF(m_width - 1, 0));
    painter->drawLine(QPointF(0, 2), QPointF(m_width - 1, 2));

    // Podwójna linia na dole peronu (odstęp 1px)
    painter->drawLine(QPointF(0, m_height - 1), QPointF(m_width - 1, m_height - 1));
    painter->drawLine(QPointF(0, m_height - 3), QPointF(m_width - 1, m_height - 3));

    // Pionowe zamknięcia po bokach
    painter->drawLine(QPointF(0, 0), QPointF(0, m_height - 1));
    painter->drawLine(QPointF(m_width - 1, 0), QPointF(m_width - 1, m_height - 1));

    // Tekst peronu dokładnie wycentrowany w ramce (odcień niebieskawy wg wzoru)
    QFont font("Arial", 8, QFont::Normal);
    font.setStyleStrategy(QFont::PreferAntialias);
    painter->setFont(font);
    painter->setPen(QColor(135, 155, 175)); // chabrowo-szary
    painter->drawText(r, Qt::AlignCenter, m_text);
}

