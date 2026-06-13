#include <QTest>

#include "models/vehicle_type_model.hpp"

namespace symulator::tools::vehicle_browser
{

class VehicleTypeModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void returnsRowsColumnsAndValues();
};

void VehicleTypeModelTest::returnsRowsColumnsAndValues()
{
    VehicleType type;
    type.uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE_TYPE, 0, 1);
    type.type_name = "ET22";
    type.family = "201E";
    type.vehicle_type = "LOCOMOTIVE";
    type.vehicle_subtype = "ELECTRIC";
    type.mass_empty_t = 84.0;
    type.length_m = 19.24;

    VehicleTypeModel model;
    model.setVehicleTypes({type});

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.columnCount(), static_cast<int>(VehicleTypeModel::COLUMN_COUNT));
    QCOMPARE(model.data(model.index(0, VehicleTypeModel::NAME_COLUMN)).toString(),
             QStringLiteral("ET22"));
    QCOMPARE(model.data(model.index(0, VehicleTypeModel::TYPE_COLUMN)).toString(),
             QStringLiteral("LOCOMOTIVE"));
    QCOMPARE(model.data(model.index(0, VehicleTypeModel::LENGTH_COLUMN)).toDouble(), 19.24);
}

}  // namespace symulator::tools::vehicle_browser

QTEST_APPLESS_MAIN(symulator::tools::vehicle_browser::VehicleTypeModelTest)

#include "test_vehicle_type_model.moc"
