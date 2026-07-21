#include <trackview/qt/track_theme.hpp>

namespace trackview
{

QBrush EbiScreenTheme::background() const
{
    return QBrush(QColor(20, 24, 28));
}

QPen EbiScreenTheme::track_pen(OccupancyState occupancy) const
{
    const QColor colour = occupancy == OccupancyState::Occupied ? QColor(220, 45, 45)
                                                                  : QColor(220, 220, 220);
    return QPen(colour, 1.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

QPen EbiScreenTheme::switch_pen(OccupancyState occupancy, bool active) const
{
    if (!active)
        return QPen(QColor(95, 105, 110), 0.8, Qt::SolidLine, Qt::RoundCap);
    const QColor colour = occupancy == OccupancyState::Occupied ? QColor(220, 45, 45)
                                                                  : QColor(220, 220, 220);
    return QPen(colour, 1.3, Qt::SolidLine, Qt::RoundCap);
}

QBrush EbiScreenTheme::signal_brush(SignalIndicationState indication) const
{
    return QBrush(indication == SignalIndicationState::Stop ? QColor(230, 35, 35)
                                                             : QColor(40, 220, 70));
}

QColor EbiScreenTheme::label_colour() const
{
    return QColor(210, 210, 210);
}

}  // namespace trackview
