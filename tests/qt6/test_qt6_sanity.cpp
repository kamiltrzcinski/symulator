#include <gtest/gtest.h>

#include <QtCore/QLibraryInfo>
#include <QtCore/QString>

TEST(Qt6Sanity, CoreApiWorks)
{
    const QString value = QStringLiteral("symulator");
    EXPECT_EQ(value.toUpper().toStdString(), "SYMULATOR");
}

TEST(Qt6Sanity, RuntimeVersionIsQt6)
{
    const auto version = QLibraryInfo::version();
    EXPECT_GE(version.majorVersion(), 6);
}
