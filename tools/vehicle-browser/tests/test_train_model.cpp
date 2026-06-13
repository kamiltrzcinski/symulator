#include <QTest>

#include "models/train_model.hpp"

namespace symulator::tools::vehicle_browser
{

class TrainModelTest final : public QObject
{
    Q_OBJECT

private slots:
    void appendsMovesAndRemoves();
};

void TrainModelTest::appendsMovesAndRemoves()
{
    Vehicle first;
    first.uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 1);
    first.pid = "A";
    first.display_name = "Vehicle A";

    Vehicle second;
    second.uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 2);
    second.pid = "B";
    second.display_name = "Vehicle B";

    TrainModel model;
    model.appendVehicle(first);
    model.appendVehicle(second);
    QCOMPARE(model.rowCount(), 2);

    QVERIFY(model.moveRows({}, 0, 1, {}, 2));
    QCOMPARE(model.vehicles().front().uid, second.uid);
    QCOMPARE(model.vehicles().back().uid, first.uid);

    QVERIFY(model.removeVehicle(0));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.vehicles().front().uid, first.uid);
}

}  // namespace symulator::tools::vehicle_browser

QTEST_APPLESS_MAIN(symulator::tools::vehicle_browser::TrainModelTest)

#include "test_train_model.moc"
