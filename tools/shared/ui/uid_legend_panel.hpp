#pragma once

#include <QWidget>

namespace symulator::tools
{

class UidLegendPanel final : public QWidget
{
    Q_OBJECT

public:
    explicit UidLegendPanel(QWidget* parent = nullptr);
};

}  // namespace symulator::tools
