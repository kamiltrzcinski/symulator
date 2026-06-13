#include "ui/train_builder_panel.hpp"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <fstream>
#include <nlohmann/json.hpp>

#include "models/train_model.hpp"
#include "registry/uid_registry.hpp"
#include "services/uid_generator_service.hpp"

namespace symulator::tools::vehicle_browser
{

TrainBuilderPanel::TrainBuilderPanel(TrainModel& model, const UidGeneratorService& generator,
                                     UidRegistry& registry, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , generator_(generator)
    , registry_(registry)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Train consist"), this));

    list_ = new QListView(this);
    list_->setModel(&model_);
    list_->setDragDropMode(QAbstractItemView::InternalMove);
    list_->setDefaultDropAction(Qt::MoveAction);
    layout->addWidget(list_);

    auto* buttons = new QHBoxLayout();
    auto* remove_button = new QPushButton(tr("Remove"), this);
    auto* save_button = new QPushButton(tr("Save Train..."), this);
    buttons->addWidget(remove_button);
    buttons->addWidget(save_button);
    layout->addLayout(buttons);

    connect(remove_button, &QPushButton::clicked, this, &TrainBuilderPanel::removeSelected);
    connect(save_button, &QPushButton::clicked, this, &TrainBuilderPanel::saveTrain);
}

UID TrainBuilderPanel::saveTrainTo(const std::filesystem::path& file, const QString& pid,
                                   const QString& display_name, const QString& category,
                                   std::uint16_t first_instance)
{
    if (model_.vehicles().empty())
    {
        throw std::runtime_error("Cannot save an empty train consist");
    }

    const UID uid = generator_.generate(UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST, 0,
                                        first_instance);
    nlohmann::json document{
        {"uid", uid.value},
        {"pID", pid.toStdString()},
        {"displayName", display_name.toStdString()},
        {"trainCategory", category.toUpper().toStdString()},
        {"vehicle_uids", nlohmann::json::array()},
    };
    for (const auto& vehicle : model_.vehicles())
    {
        document["vehicle_uids"].push_back(vehicle.uid.value);
    }

    std::filesystem::create_directories(file.parent_path());
    std::ofstream output(file);
    if (!output)
    {
        throw std::runtime_error("Cannot create train JSON file: " + file.string());
    }
    output << document.dump(2) << '\n';
    static_cast<void>(registry_.insert(uid, file));
    return uid;
}

void TrainBuilderPanel::removeSelected()
{
    static_cast<void>(model_.removeVehicle(list_->currentIndex().row()));
}

void TrainBuilderPanel::saveTrain()
{
    bool accepted = false;
    const QString pid = QInputDialog::getText(this, tr("Train number"), tr("pID:"),
                                              QLineEdit::Normal, QString(), &accepted);
    if (!accepted || pid.trimmed().isEmpty())
    {
        return;
    }

    const QString display_name =
        QInputDialog::getText(this, tr("Display name"), tr("Display name:"),
                              QLineEdit::Normal, pid, &accepted);
    if (!accepted)
    {
        return;
    }

    const QStringList categories{QStringLiteral("PASSENGER"), QStringLiteral("FREIGHT"),
                                 QStringLiteral("MAINTENANCE")};
    const QString category = QInputDialog::getItem(
        this, tr("Train category"), tr("Category:"), categories, 0, false, &accepted);
    if (!accepted)
    {
        return;
    }

    const QString file = QFileDialog::getSaveFileName(
        this, tr("Save Train JSON"), pid + QStringLiteral(".json"),
        tr("JSON files (*.json)"));
    if (file.isEmpty())
    {
        return;
    }

    try
    {
        const UID uid =
            saveTrainTo(file.toStdWString(), pid, display_name, category);
        QMessageBox::information(
            this, tr("Train saved"),
            tr("Saved collision-free Train UID %1").arg(static_cast<qulonglong>(uid.value)));
    }
    catch (const std::exception& error)
    {
        QMessageBox::critical(this, tr("Cannot save train"), QString::fromUtf8(error.what()));
    }
}

}  // namespace symulator::tools::vehicle_browser
