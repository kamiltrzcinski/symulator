#include <QTest>

#include "registry/uid_registry.hpp"
#include "registry/uid_validator.hpp"

namespace symulator::tools
{

class UidValidatorTest final : public QObject
{
    Q_OBJECT

private slots:
    void reportsOccupiedAndFreeUid();
};

void UidValidatorTest::reportsOccupiedAndFreeUid()
{
    UidRegistry registry;
    const UID occupied = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 1);
    const UID free = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 2);
    QVERIFY(registry.insert(occupied, "vehicle.json"));

    const UidValidator validator(registry);
    QVERIFY(!validator.isAvailable(occupied));
    QVERIFY(validator.isAvailable(free));
}

}  // namespace symulator::tools

QTEST_APPLESS_MAIN(symulator::tools::UidValidatorTest)

#include "test_uid_validator.moc"
