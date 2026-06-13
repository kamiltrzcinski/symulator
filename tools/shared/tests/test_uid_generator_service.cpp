#include <QTest>

#include "registry/uid_registry.hpp"
#include "registry/uid_validator.hpp"
#include "services/uid_generator_service.hpp"

namespace symulator::tools
{

class UidGeneratorServiceTest final : public QObject
{
    Q_OBJECT

private slots:
    void generatesRequestedInstance();
    void incrementsOnCollision();
    void rejectsZeroInstance();
    void throwsWhenScopeIsFull();
};

void UidGeneratorServiceTest::generatesRequestedInstance()
{
    const UidRegistry registry;
    const UidValidator validator(registry);
    const UidGeneratorService generator(validator);

    QCOMPARE(generator.generate(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 7),
             make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 7));
}

void UidGeneratorServiceTest::incrementsOnCollision()
{
    UidRegistry registry;
    const UID occupied = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 1);
    QVERIFY(registry.insert(occupied, "vehicle.json"));
    const UidValidator validator(registry);
    const UidGeneratorService generator(validator);

    QCOMPARE(generator.generate(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 1),
             make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 2));
}

void UidGeneratorServiceTest::rejectsZeroInstance()
{
    const UidRegistry registry;
    const UidValidator validator(registry);
    const UidGeneratorService generator(validator);

    QVERIFY_EXCEPTION_THROWN(
        generator.generate(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 0),
        std::invalid_argument);
}

void UidGeneratorServiceTest::throwsWhenScopeIsFull()
{
    UidRegistry registry;
    for (std::uint32_t instance = 1; instance <= 0xFFFF; ++instance)
    {
        QVERIFY(registry.insert(
            make_uid(UIDDomain::OPERATIONS, UIDKind::ALARM, 9,
                     static_cast<std::uint16_t>(instance)),
            "occupied.json"));
    }
    const UidValidator validator(registry);
    const UidGeneratorService generator(validator);

    QVERIFY_EXCEPTION_THROWN(
        generator.generate(UIDDomain::OPERATIONS, UIDKind::ALARM, 9),
        UidExhaustedException);
}

}  // namespace symulator::tools

QTEST_APPLESS_MAIN(symulator::tools::UidGeneratorServiceTest)

#include "test_uid_generator_service.moc"
