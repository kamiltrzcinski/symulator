#include <QApplication>
#include <QTemporaryDir>
#include <QTest>

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "models/train_model.hpp"
#include "registry/uid_registry.hpp"
#include "registry/uid_validator.hpp"
#include "services/uid_generator_service.hpp"
#include "ui/train_builder_panel.hpp"
#include "ui/vehicle_edit_dialog.hpp"

namespace symulator::tools::vehicle_browser
{

class EditorPersistenceTest final : public QObject
{
    Q_OBJECT

private slots:
    void vehicleEditPreservesUnknownFields();
    void trainEditPreservesUidAndUnknownFields();
};

void EditorPersistenceTest::vehicleEditPreservesUnknownFields()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const std::filesystem::path file =
        std::filesystem::path(directory.path().toStdWString()) / "vehicle.json";

    const UID type_uid =
        make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE_TYPE, 7, 1);
    const UID vehicle_uid =
        make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 7, 1);
    {
        std::ofstream output(file);
        output << nlohmann::json{
            {"uid", vehicle_uid.value},
            {"type_uid", type_uid.value},
            {"pID", "OLD"},
            {"displayName", "Old name"},
            {"customField", "keep-me"},
        }.dump(2);
    }

    VehicleType type;
    type.uid = type_uid;
    Vehicle vehicle;
    vehicle.uid = vehicle_uid;
    vehicle.type_uid = type_uid;
    vehicle.pid = "OLD";
    vehicle.display_name = "Old name";
    vehicle.source_file = file;

    UidRegistry registry;
    QVERIFY(registry.insert(vehicle_uid, file));
    UidValidator validator(registry);
    UidGeneratorService generator(validator);
    VehicleEditDialog dialog(vehicle, type, generator, validator, registry);
    dialog.setSideNumber(QStringLiteral("NEW"));
    QCOMPARE(dialog.saveToFile(file), file);

    std::ifstream input(file);
    const nlohmann::json saved = nlohmann::json::parse(input);
    QCOMPARE(saved.at("uid").get<std::uint64_t>(), vehicle_uid.value);
    QCOMPARE(saved.at("pID").get<std::string>(), std::string("NEW"));
    QCOMPARE(saved.at("customField").get<std::string>(), std::string("keep-me"));
}

void EditorPersistenceTest::trainEditPreservesUidAndUnknownFields()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const std::filesystem::path file =
        std::filesystem::path(directory.path().toStdWString()) / "train.json";

    const UID train_uid =
        make_uid(UIDDomain::ROLLING_STOCK, UIDKind::TRAIN_CONSIST, 0, 3);
    const UID vehicle_uid =
        make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 7, 2);
    {
        std::ofstream output(file);
        output << nlohmann::json{
            {"uid", train_uid.value},
            {"pID", "T1"},
            {"displayName", "Old train"},
            {"trainCategory", "PASSENGER"},
            {"vehicle_uids", nlohmann::json::array({vehicle_uid.value})},
            {"customField", 42},
        }.dump(2);
    }

    Vehicle vehicle;
    vehicle.uid = vehicle_uid;
    vehicle.pid = "V1";
    vehicle.display_name = "Vehicle 1";

    UidRegistry registry;
    QVERIFY(registry.insert(train_uid, file));
    UidValidator validator(registry);
    UidGeneratorService generator(validator);
    TrainModel model;
    TrainBuilderPanel panel(model, generator, registry);
    model.appendVehicle(vehicle);

    QCOMPARE(panel.saveTrainTo(file, QStringLiteral("T2"), QStringLiteral("New train"),
                               QStringLiteral("FREIGHT"), 1, train_uid),
             train_uid);

    std::ifstream input(file);
    const nlohmann::json saved = nlohmann::json::parse(input);
    QCOMPARE(saved.at("uid").get<std::uint64_t>(), train_uid.value);
    QCOMPARE(saved.at("pID").get<std::string>(), std::string("T2"));
    QCOMPARE(saved.at("customField").get<int>(), 42);
}

}  // namespace symulator::tools::vehicle_browser

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    symulator::tools::vehicle_browser::EditorPersistenceTest test;
    try
    {
        return QTest::qExec(&test, argc, argv);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Unhandled test exception: " << error.what() << '\n';
        return 1;
    }
}

#include "test_editor_persistence.moc"
