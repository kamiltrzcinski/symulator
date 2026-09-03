#include <QApplication>
#include "ui/thales/thales_main_window.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    symulator::client::ui::thales::ThalesMainWindow window;
    window.show();

    return app.exec();
}
