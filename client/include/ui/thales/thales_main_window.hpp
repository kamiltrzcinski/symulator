#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QTimer>

namespace symulator::client::ui::thales {

class ThalesMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ThalesMainWindow(QWidget* parent = nullptr);
    ~ThalesMainWindow() override;

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void handleProcessCommand();
    void handleSpecCommand();
    void handleSpecTimeout();

private:
    void setupUi();
    void processCommand(const QString& cmd);

    QGraphicsView* mapView_;
    QGraphicsScene* mapScene_;
    
    // UI Elements for Command 900
    QLineEdit* inputLine_;
    QLabel* statusLine_;
    QPushButton* btnProcess_; // "P" button
    QPushButton* btnSpec_;    // "SPEC" button

    // Spec authorization timer
    QTimer* specTimer_;
    bool awaitingSpec_;
    QString pendingSpecCommand_;
};

} // namespace symulator::client::ui::thales
