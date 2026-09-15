#include <trackview/qt/thales_theme.hpp>

namespace trackview {

QBrush ThalesTrackTheme::background() const {
    return QBrush(QColor(33, 33, 33)); // Dark gray background used in Thales ML8
}

QPen ThalesTrackTheme::track_pen(OccupancyState occupancy) const {
    const QColor colour = occupancy == OccupancyState::Occupied ? QColor(255, 0, 0)
                                                                : QColor(155, 155, 155);
    return QPen(colour, 4.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
}

QPen ThalesTrackTheme::switch_pen(OccupancyState occupancy, bool active) const {
    // Thales draws switches same as tracks, with 4px width.
    const QColor colour = occupancy == OccupancyState::Occupied ? QColor(255, 0, 0)
                                                                : QColor(155, 155, 155);
    // If not active (divergent), we might use a different style, but for now just basic colors
    return QPen(colour, 4.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
}

QBrush ThalesTrackTheme::signal_brush(SignalIndicationState indication) const {
    return QBrush(indication == SignalIndicationState::Stop ? QColor(255, 0, 0)
                                                            : QColor(0, 255, 0));
}

QColor ThalesTrackTheme::label_colour() const {
    return QColor(180, 180, 180);
}

} // namespace trackview
