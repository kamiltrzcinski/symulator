#include <QApplication>
#include <QTabWidget>
#include "thales/thales_window.hpp"
#include "thales/thales_browser.hpp"

// Thales ML8 Client Entry Point
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QTabWidget* tabs = new QTabWidget();

    ThalesBrowserWindow* browserWindow = new ThalesBrowserWindow();
    tabs->addTab(browserWindow, "Przegladarka Komponentow");
    
    ThalesWindow* window = new ThalesWindow();
    tabs->addTab(window, "Zgrubny Symulator");
    
    tabs->resize(1200, 800);
    tabs->show();
    
    return app.exec();
}
