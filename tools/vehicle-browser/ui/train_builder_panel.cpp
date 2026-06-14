#include "ui/train_builder_panel.hpp"

#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <utility>

#include "models/train_model.hpp"
#include "registry/uid_registry.hpp"
#include "services/uid_generator_service.hpp"

namespace symulator::tools::vehicle_browser
{

TrainBuilderPanel::TrainBuilderPanel(TrainModel& model,
                                     const UidGeneratorService& generator,
                                     UidRegistry& registry, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , generator_(generator)
    , registry_(registry)
{
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Edytor składu pociągu"), this));

    train_selector_ = new QComboBox(this);
    layout->addWidget(train_selector_);

    auto* form = new QFormLayout();
    uid_field_ = new QLineEdit(this);
    uid_field_->setReadOnly(true);
    form->addRow(tr("UID:"), uid_field_);

    pid_field_ = new QLineEdit(this);
    form->addRow(tr("Numer pociągu:"), pid_field_);

    display_name_field_ = new QLineEdit(this);
    form->addRow(tr("Nazwa:"), display_name_field_);

    category_field_ = new QComboBox(this);
    category_field_->addItems(
        {QStringLiteral("PASSENGER"), QStringLiteral("FREIGHT"),
         QStringLiteral("MAINTENANCE")});
    form->addRow(tr("Kategoria:"), category_field_);

    carrier_uid_field_ = new QLineEdit(this);
    carrier_uid_field_->setPlaceholderText(tr("Opcjonalny numeryczny UID"));
    form->addRow(tr("UID przewoźnika:"), carrier_uid_field_);
    layout->addLayout(form);

    source_label_ = new QLabel(this);
    source_label_->setWordWrap(true);
    layout->addWidget(source_label_);

    list_ = new QListView(this);
    list_->setModel(&model_);
    list_->setDragDropMode(QAbstractItemView::InternalMove);
    list_->setDefaultDropAction(Qt::MoveAction);
    layout->addWidget(list_, 1);

    auto* row_actions = new QHBoxLayout();
    remove_button_ = new QPushButton(tr("Usuń pojazd"), this);
    auto* new_button = new QPushButton(tr("Nowy skład"), this);
    row_actions->addWidget(remove_button_);
    row_actions->addWidget(new_button);
    layout->addLayout(row_actions);

    auto* save_actions = new QHBoxLayout();
    save_button_ = new QPushButton(tr("Zapisz"), this);
    save_as_button_ = new QPushButton(tr("Zapisz jako..."), this);
    save_actions->addWidget(save_button_);
    save_actions->addWidget(save_as_button_);
    layout->addLayout(save_actions);

    connect(train_selector_, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &TrainBuilderPanel::selectTrain);
    connect(new_button, &QPushButton::clicked, this, &TrainBuilderPanel::newTrain);
    connect(remove_button_, &QPushButton::clicked, this,
            &TrainBuilderPanel::removeSelected);
    connect(save_button_, &QPushButton::clicked, this, &TrainBuilderPanel::saveTrain);
    connect(save_as_button_, &QPushButton::clicked, this,
            &TrainBuilderPanel::saveTrainAs);
    connect(list_->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &TrainBuilderPanel::updateActionState);
    connect(pid_field_, &QLineEdit::textChanged, this,
            &TrainBuilderPanel::updateActionState);
    connect(&model_, &QAbstractItemModel::rowsInserted, this,
            &TrainBuilderPanel::updateActionState);
    connect(&model_, &QAbstractItemModel::rowsRemoved, this,
            &TrainBuilderPanel::updateActionState);
    connect(&model_, &QAbstractItemModel::modelReset, this,
            &TrainBuilderPanel::updateActionState);

    setAvailableData({}, {}, {});
}

void TrainBuilderPanel::setAvailableData(
    std::vector<Train> trains, std::vector<Vehicle> vehicles,
    std::filesystem::path default_output_directory)
{
    std::optional<UID> selected_uid;
    if (current_train_index_.has_value() &&
        *current_train_index_ < trains_.size())
    {
        selected_uid = trains_[*current_train_index_].uid;
    }

    trains_ = std::move(trains);
    vehicles_ = std::move(vehicles);
    default_output_directory_ = std::move(default_output_directory);

    train_selector_->blockSignals(true);
    train_selector_->clear();
    train_selector_->addItem(tr("(Nowy skład)"));
    int selected_index = 0;
    for (std::size_t index = 0; index < trains_.size(); ++index)
    {
        const Train& train = trains_[index];
        train_selector_->addItem(
            tr("%1 — %2").arg(QString::fromStdString(train.pid),
                              QString::fromStdString(train.display_name)));
        if (selected_uid.has_value() && train.uid == *selected_uid)
        {
            selected_index = static_cast<int>(index) + 1;
        }
    }
    train_selector_->setCurrentIndex(selected_index);
    train_selector_->blockSignals(false);
    selectTrain(selected_index);
}

UID TrainBuilderPanel::saveTrainTo(
    const std::filesystem::path& file, const QString& pid,
    const QString& display_name, const QString& category,
    std::uint16_t first_instance, std::optional<UID> existing_uid,
    std::optional<UID> carrier_id)
{
    if (model_.vehicles().empty())
    {
        throw std::runtime_error("Nie można zapisać pustego składu pociągu");
    }
    if (pid.trimmed().isEmpty())
    {
        throw std::runtime_error("Numer pociągu jest wymagany");
    }

    const UID uid = existing_uid.has_value()
                        ? *existing_uid
                        : generator_.generate(UIDDomain::ROLLING_STOCK,
                                              UIDKind::TRAIN_CONSIST, 0,
                                              first_instance);
    nlohmann::json document = nlohmann::json::object();
    if (existing_uid.has_value() && std::filesystem::exists(file))
    {
        std::ifstream input(file);
        if (input)
        {
            input >> document;
        }
    }
    document["uid"] = uid.value;
    document["pID"] = pid.trimmed().toStdString();
    document["displayName"] = display_name.trimmed().isEmpty()
                                  ? pid.trimmed().toStdString()
                                  : display_name.trimmed().toStdString();
    document["trainCategory"] = category.toUpper().toStdString();
    document["vehicle_uids"] = nlohmann::json::array();
    if (carrier_id.has_value())
    {
        document["carrierId"] = carrier_id->value;
    }
    else
    {
        document.erase("carrierId");
    }
    for (const auto& vehicle : model_.vehicles())
    {
        document["vehicle_uids"].push_back(vehicle.uid.value);
    }

    if (!file.parent_path().empty())
    {
        std::filesystem::create_directories(file.parent_path());
    }
    std::ofstream output(file);
    if (!output)
    {
        throw std::runtime_error("Nie można utworzyć pliku JSON składu: " +
                                 file.string());
    }
    output << document.dump(2) << '\n';
    if (!existing_uid.has_value())
    {
        static_cast<void>(registry_.insert(uid, file));
    }
    return uid;
}

void TrainBuilderPanel::selectTrain(int index)
{
    model_.clear();
    current_train_index_.reset();
    uid_field_->clear();
    pid_field_->clear();
    display_name_field_->clear();
    category_field_->setCurrentText(QStringLiteral("PASSENGER"));
    carrier_uid_field_->clear();
    source_label_->setText(tr("Nowy, niezapisany skład."));
    missing_vehicle_count_ = 0;

    if (index <= 0 || index > static_cast<int>(trains_.size()))
    {
        updateActionState();
        return;
    }

    current_train_index_ = static_cast<std::size_t>(index - 1);
    const Train& train = trains_[*current_train_index_];
    uid_field_->setText(QString::number(static_cast<qulonglong>(train.uid.value)));
    pid_field_->setText(QString::fromStdString(train.pid));
    display_name_field_->setText(QString::fromStdString(train.display_name));
    category_field_->setCurrentText(QString::fromStdString(train.train_category).toUpper());
    if (train.carrier_id.has_value())
    {
        carrier_uid_field_->setText(
            QString::number(static_cast<qulonglong>(train.carrier_id->value)));
    }

    std::vector<Vehicle> consist;
    for (const UID uid : train.vehicle_uids)
    {
        const Vehicle* vehicle = findVehicle(uid);
        if (vehicle != nullptr)
        {
            consist.push_back(*vehicle);
        }
        else
        {
            ++missing_vehicle_count_;
        }
    }
    model_.setVehicles(std::move(consist));

    QString source =
        tr("Plik: %1").arg(QString::fromStdWString(train.source_file.wstring()));
    if (missing_vehicle_count_ > 0)
    {
        source += tr("\nUwaga: nie znaleziono %1 pojazdów wskazanych przez skład.")
                      .arg(static_cast<qulonglong>(missing_vehicle_count_));
    }
    source_label_->setText(source);
    updateActionState();
}

void TrainBuilderPanel::newTrain()
{
    train_selector_->setCurrentIndex(0);
    pid_field_->setFocus();
}

void TrainBuilderPanel::removeSelected()
{
    static_cast<void>(model_.removeVehicle(list_->currentIndex().row()));
}

void TrainBuilderPanel::saveTrain()
{
    saveCurrent(false);
}

void TrainBuilderPanel::saveTrainAs()
{
    saveCurrent(true);
}

void TrainBuilderPanel::saveCurrent(bool save_as)
{
    try
    {
        if (missing_vehicle_count_ > 0)
        {
            throw std::runtime_error(
                "Nie można zapisać składu, dopóki nie zostaną wczytane wszystkie "
                "wskazane pojazdy");
        }
        const std::optional<UID> carrier_id = parsedCarrierUid();
        std::filesystem::path file;
        std::optional<UID> existing_uid;

        if (!save_as && current_train_index_.has_value())
        {
            const Train& train = trains_[*current_train_index_];
            file = train.source_file;
            existing_uid = train.uid;
        }
        else
        {
            const QString suggested =
                QString::fromStdWString((default_output_directory_ /
                                         (pid_field_->text().trimmed().toStdString() +
                                          ".json"))
                                            .wstring());
            const QString selected = QFileDialog::getSaveFileName(
                this, tr("Zapisz skład"), suggested, tr("Pliki JSON (*.json)"));
            if (selected.isEmpty())
            {
                return;
            }
            file = selected.toStdWString();
        }

        const UID uid = saveTrainTo(
            file, pid_field_->text(), display_name_field_->text(),
            category_field_->currentText(), 1, existing_uid, carrier_id);
        QMessageBox::information(
            this, tr("Skład zapisany"),
            tr("Zapisano skład z UID %1:\n%2")
                .arg(static_cast<qulonglong>(uid.value))
                .arg(QString::fromStdWString(file.wstring())));
        emit trainSaved(QString::fromStdWString(file.wstring()));
    }
    catch (const std::exception& error)
    {
        QMessageBox::critical(this, tr("Nie można zapisać składu"),
                              QString::fromUtf8(error.what()));
    }
}

std::optional<UID> TrainBuilderPanel::parsedCarrierUid() const
{
    const QString text = carrier_uid_field_->text().trimmed();
    if (text.isEmpty())
    {
        return std::nullopt;
    }

    bool valid = false;
    const qulonglong value = text.toULongLong(&valid);
    if (!valid || value == 0)
    {
        throw std::invalid_argument("UID przewoźnika musi być dodatnią liczbą");
    }
    return UID{static_cast<std::uint64_t>(value)};
}

const Vehicle* TrainBuilderPanel::findVehicle(UID uid) const
{
    for (const Vehicle& vehicle : vehicles_)
    {
        if (vehicle.uid == uid)
        {
            return &vehicle;
        }
    }
    return nullptr;
}

void TrainBuilderPanel::updateActionState()
{
    const bool has_rows = !model_.vehicles().empty();
    const bool has_pid = !pid_field_->text().trimmed().isEmpty();
    const bool data_complete = missing_vehicle_count_ == 0;
    remove_button_->setEnabled(list_->currentIndex().isValid());
    save_button_->setEnabled(has_rows && has_pid && data_complete);
    save_as_button_->setEnabled(has_rows && has_pid && data_complete);
}

}  // namespace symulator::tools::vehicle_browser
