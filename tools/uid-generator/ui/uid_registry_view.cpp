#include "ui/uid_registry_view.hpp"

#include <QHeaderView>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

#include "registry/uid_registry.hpp"

namespace symulator::tools::uid_generator
{

UidRegistryView::UidRegistryView(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    table_ = new QTableView(this);
    model_ = new QStandardItemModel(this);
    model_->setHorizontalHeaderLabels(
        {tr("UID hex"), tr("Domain"), tr("Kind"), tr("SCOPE"), tr("INSTANCE"),
         tr("Source file")});
    table_->setModel(model_);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSortingEnabled(true);
    table_->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(table_);
}

void UidRegistryView::setRegistry(const UidRegistry& registry)
{
    model_->removeRows(0, model_->rowCount());
    for (const auto& entry : registry.entries())
    {
        const UID uid = entry.uid;
        QList<QStandardItem*> row;
        row << new QStandardItem(
            QStringLiteral("0x%1")
                .arg(static_cast<qulonglong>(uid.value), 12, 16, QLatin1Char('0'))
                .toUpper());
        row << new QStandardItem(QString::number(static_cast<int>(uid_domain(uid))));
        row << new QStandardItem(QString::number(static_cast<int>(uid_kind(uid))));
        row << new QStandardItem(QString::number(uid_scope(uid)));
        row << new QStandardItem(QString::number(uid_instance(uid)));
        row << new QStandardItem(QString::fromStdWString(entry.source_file.wstring()));
        model_->appendRow(row);
    }
    table_->resizeColumnsToContents();
}

}  // namespace symulator::tools::uid_generator
