#include "thales/graphics/thales_header_item.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QDateTime>
#include <QTimer>
#include <cmath>

// ============================================================================
ThalesHeaderGraphic::ThalesHeaderGraphic(QGraphicsItem* parent, engine::core::UID uid)
    : ThalesElementGraphic(parent, uid) {
}

QRectF ThalesHeaderGraphic::boundingRect() const {
    return QRectF(0, 0, 340, 65);
}

void ThalesHeaderGraphic::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    // Kwadrat [P]
    painter->fillRect(QRectF(0, 0, 18, 18), QColor(0, 50, 220));
    QFont fontP("Arial", 9, QFont::Bold);
    painter->setFont(fontP);
    painter->setPen(Qt::white);
    painter->drawText(QRectF(0, 0, 18, 18), Qt::AlignCenter, "P");

    // Bloki [R] [G] [B]
    painter->fillRect(QRectF(24, 0, 18, 18), QColor(220, 0, 0));
    painter->drawText(QRectF(24, 0, 18, 18), Qt::AlignCenter, "R");

    painter->fillRect(QRectF(44, 0, 18, 18), QColor(0, 180, 0));
    painter->drawText(QRectF(44, 0, 18, 18), Qt::AlignCenter, "G");

    painter->fillRect(QRectF(64, 0, 18, 18), QColor(0, 50, 220));
    painter->drawText(QRectF(64, 0, 18, 18), Qt::AlignCenter, "B");

    // Zegar czasu rzeczywistego (odświeżany co sekundę)
    QDateTime now = QDateTime::currentDateTime();
    static const char* const days[] = {"PN", "WT", "ŚR", "CZ", "PT", "SO", "ND"};
    static const char* const months[] = {"STY", "LUT", "MAR", "KWI", "MAJ", "CZE", "LIP", "SIE", "WRZ", "PAŹ", "LIS", "GRU"};
    
    int dayOfWeek = now.date().dayOfWeek() - 1; // 1 = Monday -> 0
    if (dayOfWeek < 0 || dayOfWeek > 6) dayOfWeek = 0;
    int month = now.date().month() - 1;
    if (month < 0 || month > 11) month = 0;

    QString dateStr = QString("%1, %2-%3-%4")
        .arg(days[dayOfWeek])
        .arg(now.date().day(), 2, 10, QChar('0'))
        .arg(months[month])
        .arg(now.date().year());
    QString timeStr = now.time().toString("hh:mm:ss");

    QFont clockFont("Arial", 8, QFont::Normal);
    painter->setFont(clockFont);
    painter->setPen(QColor(180, 180, 180));
    painter->drawText(QRectF(90, 0, 140, 14), Qt::AlignRight, dateStr);
    painter->drawText(QRectF(90, 16, 140, 14), Qt::AlignRight, timeStr);

    // Paski kolorów nad logo
    painter->fillRect(QRectF(255, 2, 10, 3), QColor(0, 100, 255));
    painter->fillRect(QRectF(268, 2, 10, 3), Qt::white);
    painter->fillRect(QRectF(281, 2, 10, 3), QColor(255, 0, 0));

    // Logo THALES
    QFont logoFont("Arial", 10, QFont::Bold);
    painter->setFont(logoFont);
    painter->setPen(QColor(0, 80, 220));
    painter->drawText(QRectF(245, 12, 85, 16), Qt::AlignCenter, "THALES");

    // Logo RSS
    QFont rssFont("Arial", 11, QFont::Bold);
    painter->setFont(rssFont);
    painter->setPen(QColor(0, 255, 0));
    painter->drawText(QRectF(245, 28, 85, 18), Qt::AlignCenter, "RSS");
}

