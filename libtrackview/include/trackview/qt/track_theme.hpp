#pragma once

#include <trackview/core/runtime_state.hpp>

#include <QBrush>
#include <QColor>
#include <QPen>

namespace trackview
{

class ITrackTheme
{
public:
    virtual ~ITrackTheme() = default;
    [[nodiscard]] virtual QBrush background() const = 0;
    [[nodiscard]] virtual QPen track_pen(OccupancyState occupancy) const = 0;
    [[nodiscard]] virtual QPen switch_pen(OccupancyState occupancy,
                                          bool active) const = 0;
    [[nodiscard]] virtual QBrush signal_brush(SignalIndicationState indication) const = 0;
    [[nodiscard]] virtual QColor label_colour() const = 0;
};

class EbiScreenTheme final : public ITrackTheme
{
public:
    [[nodiscard]] QBrush background() const override;
    [[nodiscard]] QPen track_pen(OccupancyState occupancy) const override;
    [[nodiscard]] QPen switch_pen(OccupancyState occupancy,
                                  bool active) const override;
    [[nodiscard]] QBrush signal_brush(SignalIndicationState indication) const override;
    [[nodiscard]] QColor label_colour() const override;
};

}  // namespace trackview
