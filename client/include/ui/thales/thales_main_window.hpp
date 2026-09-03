#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

class IThalesCommandProcessor;

class ThalesMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ThalesMainWindow(IThalesCommandProcessor* processor, QWidget* parent = nullptr);
    ~ThalesMainWindow() override = default;

private slots:
    void onInputReturnPressed();
    void onSpecButtonClicked();
    
    // Sloty obsługujące sygnały z procesora komend
    void onCommandAccepted();
    void onCommandRejected(const QString& reason);
    void onAuthorizationRequired();
    void onAuthorizationTimeout();

private:
    void setupUi();

    IThalesCommandProcessor* m_processor;

    QLineEdit* m_inputField;
    QPushButton* m_specButton;
    QTextEdit* m_displayArea;
};
