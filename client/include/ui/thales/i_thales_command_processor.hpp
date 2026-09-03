#pragma once
#include <QObject>
#include <QString>

/**
 * @brief Interfejs procesora komend dla Thales ML8
 * Umożliwia odwrócenie zależności (DIP) i łatwiejsze testowanie.
 */
class IThalesCommandProcessor : public QObject {
    Q_OBJECT
public:
    explicit IThalesCommandProcessor(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IThalesCommandProcessor() = default;

    virtual void processInput(const QString& input) = 0;
    virtual void confirmAuthorization() = 0;

signals:
    void commandAccepted();
    void commandRejected(const QString& reason);
    void authorizationRequired();
    void authorizationTimeout();
};
