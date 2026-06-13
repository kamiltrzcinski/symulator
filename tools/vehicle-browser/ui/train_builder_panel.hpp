#pragma once

#include <QWidget>

#include <filesystem>

#include "domain/uid_types.hpp"

class QListView;

namespace symulator::tools
{
class UidGeneratorService;
class UidRegistry;
}

namespace symulator::tools::vehicle_browser
{

class TrainModel;

class TrainBuilderPanel final : public QWidget
{
    Q_OBJECT

public:
    TrainBuilderPanel(TrainModel& model, const UidGeneratorService& generator,
                      UidRegistry& registry, QWidget* parent = nullptr);

    [[nodiscard]] UID saveTrainTo(const std::filesystem::path& file, const QString& pid,
                                  const QString& display_name, const QString& category,
                                  std::uint16_t first_instance = 1);

private slots:
    void removeSelected();
    void saveTrain();

private:
    TrainModel& model_;
    const UidGeneratorService& generator_;
    UidRegistry& registry_;
    QListView* list_ = nullptr;
};

}  // namespace symulator::tools::vehicle_browser
