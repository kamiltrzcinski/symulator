#include "ui/uid_form.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "domain/uid_legend_table.hpp"

namespace symulator::tools::uid_generator
{

UidForm::UidForm(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    domain_combo_ = new QComboBox(this);
    for (const UIDDomain domain : kKnownUidDomains)
    {
        for (const auto& entry : kUidLegendEntries)
        {
            if (entry.domain == domain)
            {
                domain_combo_->addItem(
                    QString::fromUtf8(entry.domain_name.data(), entry.domain_name.size()),
                    static_cast<int>(domain));
                break;
            }
        }
    }
    form->addRow(tr("Domain:"), domain_combo_);

    kind_combo_ = new QComboBox(this);
    form->addRow(tr("Kind:"), kind_combo_);

    scope_spin_ = new QSpinBox(this);
    scope_spin_->setRange(0, 0xFFFF);
    scope_spin_->setDisplayIntegerBase(10);
    form->addRow(tr("SCOPE:"), scope_spin_);

    instance_spin_ = new QSpinBox(this);
    instance_spin_->setRange(0, 0xFFFF);
    instance_spin_->setValue(1);
    form->addRow(tr("First INSTANCE:"), instance_spin_);

    layout->addLayout(form);

    advisory_label_ = new QLabel(this);
    advisory_label_->setStyleSheet(QStringLiteral("color: #7a5a00;"));
    advisory_label_->setWordWrap(true);
    layout->addWidget(advisory_label_);

    error_label_ = new QLabel(this);
    error_label_->setStyleSheet(QStringLiteral("color: #9b1c1c;"));
    error_label_->setWordWrap(true);
    layout->addWidget(error_label_);

    generate_button_ = new QPushButton(tr("Generate"), this);
    layout->addWidget(generate_button_);
    layout->addStretch();

    connect(domain_combo_, &QComboBox::currentIndexChanged, this, &UidForm::updateKinds);
    connect(domain_combo_, &QComboBox::currentIndexChanged, this,
            &UidForm::updateValidation);
    connect(kind_combo_, &QComboBox::currentIndexChanged, this, &UidForm::updateValidation);
    connect(scope_spin_, &QSpinBox::valueChanged, this, &UidForm::updateValidation);
    connect(instance_spin_, &QSpinBox::valueChanged, this, &UidForm::updateValidation);
    connect(generate_button_, &QPushButton::clicked, this, &UidForm::requestGeneration);

    updateKinds();
    updateValidation();
}

void UidForm::showError(const QString& message, bool block_generation)
{
    error_label_->setText(message);
    generation_blocked_ = block_generation;
    if (block_generation)
    {
        blocked_domain_ = domain_combo_->currentData().toInt();
        blocked_kind_ = kind_combo_->currentData().toInt();
        blocked_scope_ = scope_spin_->value();
    }
    generate_button_->setEnabled(!block_generation && instance_spin_->value() != 0);
}

void UidForm::clearError()
{
    error_label_->clear();
    generation_blocked_ = false;
    blocked_domain_.reset();
    blocked_kind_.reset();
    blocked_scope_.reset();
    updateValidation();
}

void UidForm::updateKinds()
{
    const auto domain = static_cast<UIDDomain>(domain_combo_->currentData().toInt());
    kind_combo_->clear();
    for (const auto& entry : kUidLegendEntries)
    {
        if (entry.domain == domain)
        {
            kind_combo_->addItem(
                QString::fromUtf8(entry.kind_name.data(), entry.kind_name.size()),
                static_cast<int>(entry.kind));
        }
    }
}

void UidForm::updateValidation()
{
    const int domain_value = domain_combo_->currentData().toInt();
    const int kind_value = kind_combo_->currentData().toInt();
    const int scope_value = scope_spin_->value();
    const bool same_exhausted_bucket =
        blocked_domain_ == domain_value && blocked_kind_ == kind_value &&
        blocked_scope_ == scope_value;

    generation_blocked_ = same_exhausted_bucket;
    if (!same_exhausted_bucket)
    {
        error_label_->clear();
        blocked_domain_.reset();
        blocked_kind_.reset();
        blocked_scope_.reset();
    }

    const auto domain = static_cast<UIDDomain>(domain_value);
    if (instance_spin_->value() == 0)
    {
        error_label_->setText(tr("INSTANCE must be in range 0x0001-0xFFFF"));
    }

    if (domain == UIDDomain::ROLLING_STOCK && scope_spin_->value() == 0)
    {
        advisory_label_->setText(
            tr("SCOPE=0 means global - ensure this is intentional"));
    }
    else
    {
        advisory_label_->clear();
    }

    generate_button_->setEnabled(instance_spin_->value() != 0 && !generation_blocked_);
}

void UidForm::requestGeneration()
{
    emit generateRequested(domain_combo_->currentData().toInt(),
                           kind_combo_->currentData().toInt(), scope_spin_->value(),
                           instance_spin_->value());
}

}  // namespace symulator::tools::uid_generator
