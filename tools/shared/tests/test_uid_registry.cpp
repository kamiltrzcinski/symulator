#include <QTest>

#include "registry/uid_registry.hpp"

namespace symulator::tools
{

class UidRegistryTest final : public QObject
{
    Q_OBJECT

private slots:
    void insertsAndFindsUid();
    void rejectsDuplicate();
};

void UidRegistryTest::insertsAndFindsUid()
{
    UidRegistry registry;
    const UID uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 1);

    QVERIFY(registry.insert(uid, "first.json"));
    QVERIFY(registry.contains(uid));
    QCOMPARE(registry.size(), std::size_t{1});
    QCOMPARE(registry.sourceFile(uid), std::optional<std::filesystem::path>{"first.json"});
}

void UidRegistryTest::rejectsDuplicate()
{
    UidRegistry registry;
    const UID uid = make_uid(UIDDomain::ROLLING_STOCK, UIDKind::VEHICLE, 0, 1);

    QVERIFY(registry.insert(uid, "first.json"));
    QVERIFY(!registry.insert(uid, "second.json"));
    QCOMPARE(registry.size(), std::size_t{1});
    QCOMPARE(registry.sourceFile(uid), std::optional<std::filesystem::path>{"first.json"});
}

}  // namespace symulator::tools

QTEST_APPLESS_MAIN(symulator::tools::UidRegistryTest)

#include "test_uid_registry.moc"
