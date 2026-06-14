#include "ui/uid_legend_panel.hpp"

#include <QAbstractItemView>
#include <QFontDatabase>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>

#include "domain/uid_legend_table.hpp"

namespace symulator::tools
{

namespace
{

[[nodiscard]] QString hexByte(std::uint8_t value)
{
    return QStringLiteral("0x%1").arg(value, 2, 16, QLatin1Char('0')).toUpper();
}

}  // namespace

UidLegendPanel::UidLegendPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);

    auto* title = new QLabel(tr("Układ bitów UID"), this);
    QFont title_font = title->font();
    title_font.setBold(true);
    title_font.setPointSize(title_font.pointSize() + 2);
    title->setFont(title_font);
    layout->addWidget(title);

    auto* diagram = new QLabel(
        QStringLiteral(
            "63        48 47    40 39    32 31             16 15              0\n"
            "+------------+--------+--------+----------------+------------------+\n"
            "| zarezerw.  | DOMENA | RODZAJ |     ZAKRES     |    INSTANCJA     |\n"
            "|  16 bitów  | 8 bit. | 8 bit. |    16 bitów    |     16 bitów     |\n"
            "+------------+--------+--------+----------------+------------------+"),
        this);
    diagram->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    diagram->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(diagram);

    auto* table = new QTableWidget(static_cast<int>(kUidLegendEntries.size()), 5, this);
    table->setHorizontalHeaderLabels(
        {tr("Domena"), tr("Domena hex"), tr("Rodzaj"), tr("Rodzaj hex"),
         tr("Znaczenie ZAKRESU")});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);

    for (std::size_t index = 0; index < kUidLegendEntries.size(); ++index)
    {
        const auto& entry = kUidLegendEntries[index];
        const int row = static_cast<int>(index);
        table->setItem(row, 0,
                       new QTableWidgetItem(QString::fromUtf8(entry.domain_name.data(),
                                                              entry.domain_name.size())));
        table->setItem(row, 1, new QTableWidgetItem(hexByte(uidDomainValue(entry))));
        table->setItem(row, 2,
                       new QTableWidgetItem(
                           QString::fromUtf8(entry.kind_name.data(), entry.kind_name.size())));
        table->setItem(row, 3, new QTableWidgetItem(hexByte(uidKindValue(entry))));
        table->setItem(
            row, 4,
            new QTableWidgetItem(QString::fromUtf8(entry.scope_semantics.data(),
                                                   entry.scope_semantics.size())));
    }

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    layout->addWidget(table, 1);
}

}  // namespace symulator::tools
