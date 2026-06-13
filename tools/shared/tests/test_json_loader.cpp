#include <QTemporaryDir>
#include <QTest>

#include <fstream>

#include "data/directory_data_source.hpp"
#include "data/json_loader.hpp"

namespace symulator::tools
{

class JsonLoaderTest final : public QObject
{
    Q_OBJECT

private slots:
    void mapsVehicleType();
    void mapsVehicle();
    void mapsTrain();
    void directorySourceSkipsMalformedJson();
};

namespace
{

[[nodiscard]] std::filesystem::path writeFixture(QTemporaryDir& directory,
                                                const std::string& name,
                                                const std::string& contents)
{
    const std::filesystem::path path =
        std::filesystem::path(directory.path().toStdWString()) / name;
    std::ofstream output(path);
    output << contents;
    return path;
}

}  // namespace

void JsonLoaderTest::mapsVehicleType()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = writeFixture(
        directory, "type.json",
        R"({"uid":1103806595073,"typeName":"Test","vehicleType":"LOCOMOTIVE",)"
        R"("vehicleSubtype":"ELECTRIC","lengthM":19.2,"axleCount":6,)"
        R"("massEmptyT":84.0,"massGrossT":90.0,"maxSpeedKmh":125,)"
        R"("brakingLambdaPct":100,"powerKW":2000.0})");

    const VehicleType type = JsonLoader::loadVehicleType(path);
    QCOMPARE(type.uid.value, std::uint64_t{1103806595073});
    QCOMPARE(type.type_name, std::string{"Test"});
    QCOMPARE(type.vehicle_type, std::string{"LOCOMOTIVE"});
    QCOMPARE(type.length_m, 19.2);
    QCOMPARE(type.power_kw, std::optional<double>{2000.0});
}

void JsonLoaderTest::mapsVehicle()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = writeFixture(
        directory, "vehicle.json",
        R"({"uid":1108101562369,"type_uid":1103806595073,)"
        R"("pID":"TEST-001","displayName":"Test 001","tractionStatus":"OPERATIONAL"})");

    const Vehicle vehicle = JsonLoader::loadVehicle(path);
    QCOMPARE(vehicle.uid.value, std::uint64_t{1108101562369});
    QCOMPARE(vehicle.type_uid.value, std::uint64_t{1103806595073});
    QCOMPARE(vehicle.pid, std::string{"TEST-001"});
    QCOMPARE(vehicle.traction_status, std::optional<std::string>{"OPERATIONAL"});
}

void JsonLoaderTest::mapsTrain()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const auto path = writeFixture(
        directory, "train.json",
        R"({"uid":1112396529665,"pID":"T1","displayName":"Train 1",)"
        R"("trainCategory":"FREIGHT","vehicle_uids":[1108101562369]})");

    const Train train = JsonLoader::loadTrain(path);
    QCOMPARE(train.uid.value, std::uint64_t{1112396529665});
    QCOMPARE(train.pid, std::string{"T1"});
    QCOMPARE(train.train_category, std::string{"FREIGHT"});
    QCOMPARE(train.vehicle_uids.size(), std::size_t{1});
    QCOMPARE(train.vehicle_uids.front().value, std::uint64_t{1108101562369});
}

void JsonLoaderTest::directorySourceSkipsMalformedJson()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    static_cast<void>(writeFixture(directory, "broken.json", "{not valid json"));
    static_cast<void>(writeFixture(
        directory, "type.json",
        R"({"uid":1103806595073,"typeName":"Test","vehicleType":"LOCOMOTIVE",)"
        R"("lengthM":19.2,"axleCount":6,"massEmptyT":84.0,)"
        R"("maxSpeedKmh":125,"brakingLambdaPct":100})"));

    const DirectoryDataSource source(
        std::filesystem::path(directory.path().toStdWString()));
    const auto types = source.loadVehicleTypes();

    QCOMPARE(types.size(), std::size_t{1});
    QCOMPARE(types.front().type_name, std::string{"Test"});
}

}  // namespace symulator::tools

QTEST_APPLESS_MAIN(symulator::tools::JsonLoaderTest)

#include "test_json_loader.moc"
