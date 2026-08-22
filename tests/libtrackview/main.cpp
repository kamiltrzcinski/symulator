#include <QApplication>

#include <cstdio>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    // Unbuffered so CI captures progress even if the process is later killed
    // (e.g. by ctest's TIMEOUT) before libc would otherwise flush on exit.
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication application(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
