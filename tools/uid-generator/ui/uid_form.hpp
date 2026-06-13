#pragma once

#include <QWidget>

#include <optional>

class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;

namespace symulator::tools::uid_generator
{

class UidForm final : public QWidget
{
    Q_OBJECT

public:
    explicit UidForm(QWidget* parent = nullptr);

    void showError(const QString& message, bool block_generation);
    void clearError();

signals:
    void generateRequested(int domain, int kind, int scope, int instance);

private slots:
    void updateKinds();
    void updateValidation();
    void requestGeneration();

private:
    QComboBox* domain_combo_ = nullptr;
    QComboBox* kind_combo_ = nullptr;
    QSpinBox* scope_spin_ = nullptr;
    QSpinBox* instance_spin_ = nullptr;
    QLabel* advisory_label_ = nullptr;
    QLabel* error_label_ = nullptr;
    QPushButton* generate_button_ = nullptr;
    bool generation_blocked_ = false;
    std::optional<int> blocked_domain_;
    std::optional<int> blocked_kind_;
    std::optional<int> blocked_scope_;
};

}  // namespace symulator::tools::uid_generator
