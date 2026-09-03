#pragma once
#include "i_thales_command_processor.hpp"
#include <QTimer>
#include <QStringList>

class ThalesCommandProcessor : public IThalesCommandProcessor {
    Q_OBJECT
public:
    explicit ThalesCommandProcessor(QObject* parent = nullptr);
    ~ThalesCommandProcessor() override = default;

    void processInput(const QString& input) override;
    void confirmAuthorization() override;

private slots:
    void handleAuthorizationTimeout();

private:
    bool isCommandValid(const QString& input) const;

    QTimer* m_specTimer;
    
    // Zastąpienie magic number czytelną stałą
    static constexpr int kSpecTimeoutMs = 20000; 
};
