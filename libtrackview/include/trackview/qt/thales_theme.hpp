#pragma once

#include <trackview/qt/track_theme.hpp>

namespace trackview {

class ThalesTrackTheme final : public ITrackTheme {
public:
    [[nodiscard]] QBrush background() const override;
    [[nodiscard]] QPen track_pen(OccupancyState occupancy) const override;
    [[nodiscard]] QPen switch_pen(OccupancyState occupancy, bool active) const override;
    [[nodiscard]] QBrush signal_brush(SignalIndicationState indication) const override;
    [[nodiscard]] QColor label_colour() const override;
};

} // namespace trackview
