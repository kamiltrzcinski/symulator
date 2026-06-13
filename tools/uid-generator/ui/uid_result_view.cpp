#include "ui/uid_result_view.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "services/clipboard_service.hpp"

namespace symulator::tools::uid_generator
{

UidResultView::UidResultView(const UidClipboardService& clipboard, QWidget* parent)
    : QWidget(parent)
    , clipboard_(clipboard)
{
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    decimal_value_ = new QLabel(tr("No UID generated"), this);
    decimal_value_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(tr("Decimal:"), decimal_value_);

    hex_value_ = new QLabel(QStringLiteral("-"), this);
    hex_value_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    form->addRow(tr("Hex:"), hex_value_);
    layout->addLayout(form);

    copy_button_ = new QPushButton(tr("Copy decimal UID"), this);
    copy_button_->setEnabled(false);
    layout->addWidget(copy_button_);
    layout->addStretch();

    connect(copy_button_, &QPushButton::clicked, this, &UidResultView::copyUid);
}

void UidResultView::showUid(UID uid)
{
    uid_ = uid;
    decimal_value_->setText(QString::number(static_cast<qulonglong>(uid.value)));
    hex_value_->setText(
        QStringLiteral("0x%1").arg(static_cast<qulonglong>(uid.value), 12, 16, QLatin1Char('0'))
            .toUpper());
    copy_button_->setEnabled(true);
}

UID UidResultView::displayedUid() const noexcept
{
    return uid_;
}

void UidResultView::copyUid()
{
    if (uid_.value != 0)
    {
        clipboard_.copyUid(uid_);
    }
}

}  // namespace symulator::tools::uid_generator
