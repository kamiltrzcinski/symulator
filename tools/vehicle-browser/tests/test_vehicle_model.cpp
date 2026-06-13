#include <QTest>

#include "models/vehicle_model.hpp"

namespace symulator::tools::vehicle_browser
{

class VehicleModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void filtersAndClearsFilter();
};

void VehicleModelTest::filtersAndClearsFilter()
{
    const UID type_a = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE_TYPE, 0, 1);
    const UID type_b = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE_TYPE, 0, 2);

    Vehicle first;
    first.uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 1);
    first.type_uid = type_a;
    first.pid = "A";
    first.display_name = "Vehicle A";

    Vehicle second;
    second.uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 2);
    second.type_uid = type_b;
    second.pid = "B";
    second.display_name = "Vehicle B";

    VehicleModel model;
    model.setVehicles({first, second});
    QCOMPARE(model.rowCount(), 2);

    model.setFilterVehicleType(type_a);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, VehicleModel::SIDE_NUMBER_COLUMN)).toString(),
             QStringLiteral("A"));

    model.setFilterVehicleType(std::nullopt);
    QCOMPARE(model.rowCount(), 2);
}

}  // namespace symulator::tools::vehicle_browser

QTEST_APPLESS_MAIN(symulator::tools::vehicle_browser::VehicleModelTest)

#include "test_vehicle_model.moc"
