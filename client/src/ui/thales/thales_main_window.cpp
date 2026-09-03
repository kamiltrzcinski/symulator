#include "thales_main_window.hpp"
#include "i_thales_command_processor.hpp"

#include <QWidget>

ThalesMainWindow::ThalesMainWindow(IThalesCommandProcessor* processor, QWidget* parent)
    : QMainWindow(parent),
      m_processor(processor) {
    
    setupUi();

    if (m_processor) {
        // Zapewnienie komunikacji między kontrolerem (procesorem) a widokiem za pomocą sygnałów
        connect(m_processor, &IThalesCommandProcessor::commandAccepted, this, &ThalesMainWindow::onCommandAccepted);
        connect(m_processor, &IThalesCommandProcessor::commandRejected, this, &ThalesMainWindow::onCommandRejected);
        connect(m_processor, &IThalesCommandProcessor::authorizationRequired, this, &ThalesMainWindow::onAuthorizationRequired);
        connect(m_processor, &IThalesCommandProcessor::authorizationTimeout, this, &ThalesMainWindow::onAuthorizationTimeout);
    }
}

void ThalesMainWindow::setupUi() {
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    m_displayArea = new QTextEdit(this);
    m_displayArea->setReadOnly(true);
    m_displayArea->setObjectName("displayArea"); // Identyfikator dla QSS

    m_inputField = new QLineEdit(this);
    m_inputField->setObjectName("inputField");

    m_specButton = new QPushButton("SPEC", this);
    m_specButton->setObjectName("specButton");
    m_specButton->setEnabled(false); // Domyślnie wyłączony, włączany przez sygnał z procesora

    layout->addWidget(m_displayArea);
    layout->addWidget(m_inputField);
    layout->addWidget(m_specButton);

    setCentralWidget(centralWidget);
    setWindowTitle("Thales ML8 Client");
    resize(600, 400);

    connect(m_inputField, &QLineEdit::returnPressed, this, &ThalesMainWindow::onInputReturnPressed);
    connect(m_specButton, &QPushButton::clicked, this, &ThalesMainWindow::onSpecButtonClicked);
}

void ThalesMainWindow::onInputReturnPressed() {
    QString input = m_inputField->text();
    if (input.isEmpty()) return;

    m_displayArea->append("> " + input);
    m_inputField->clear();
    
    if (m_processor) {
        m_processor->processInput(input); // Przekazanie logiki domenowej do procesora (SRP)
    }
}

void ThalesMainWindow::onSpecButtonClicked() {
    m_specButton->setEnabled(false);
    
    if (m_processor) {
        m_processor->confirmAuthorization();
    }
}

void ThalesMainWindow::onCommandAccepted() {
    m_displayArea->append("-> Sukces: Komenda wykonana.");
}

void ThalesMainWindow::onCommandRejected(const QString& reason) {
    m_displayArea->append("-> Błąd: " + reason);
}

void ThalesMainWindow::onAuthorizationRequired() {
    m_specButton->setEnabled(true);
    m_displayArea->append("-> Wymagana autoryzacja (SPEC). Masz 20 sekund.");
}

void ThalesMainWindow::onAuthorizationTimeout() {
    m_specButton->setEnabled(false);
    m_displayArea->append("-> Czas na autoryzację SPEC minął.");
}
