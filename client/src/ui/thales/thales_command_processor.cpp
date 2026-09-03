#include "thales_command_processor.hpp"

ThalesCommandProcessor::ThalesCommandProcessor(QObject* parent)
    : IThalesCommandProcessor(parent),
      m_specTimer(new QTimer(this)) {
    
    m_specTimer->setSingleShot(true);
    connect(m_specTimer, &QTimer::timeout, this, &ThalesCommandProcessor::handleAuthorizationTimeout);
}

void ThalesCommandProcessor::processInput(const QString& input) {
    if (!isCommandValid(input)) {
        emit commandRejected("Nieprawidłowy format komendy.");
        return;
    }

    QString cmd = input.trimmed().toUpper();
    
    // Zastąpienie naiwnego contains() bezpieczniejszym sprawdzeniem StartsWith lub Exact (OCP)
    // Zamiast cmd.contains("DPZ"), co mogłoby zaakceptować "ZLEDPZ"
    if (cmd == "DPZ" || cmd.startsWith("DPZ ")) {
        emit authorizationRequired();
        m_specTimer->start(kSpecTimeoutMs);
    } else if (cmd == "HELP") {
        emit commandAccepted(); // przykładowa inna komenda
    } else {
        emit commandRejected("Nieznana komenda.");
    }
}

void ThalesCommandProcessor::confirmAuthorization() {
    if (m_specTimer->isActive()) {
        m_specTimer->stop();
        emit commandAccepted();
    } else {
        emit commandRejected("Brak oczekującej autoryzacji.");
    }
}

void ThalesCommandProcessor::handleAuthorizationTimeout() {
    emit authorizationTimeout();
}

bool ThalesCommandProcessor::isCommandValid(const QString& input) const {
    return !input.trimmed().isEmpty();
}
