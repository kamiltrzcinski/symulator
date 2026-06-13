#pragma once

#include <QWidget>

class QStandardItemModel;
class QTableView;

namespace symulator::tools
{
class UidRegistry;
}

namespace symulator::tools::uid_generator
{

class UidRegistryView final : public QWidget
{
    Q_OBJECT

public:
    explicit UidRegistryView(QWidget* parent = nullptr);

    void setRegistry(const UidRegistry& registry);

private:
    QTableView* table_ = nullptr;
    QStandardItemModel* model_ = nullptr;
};

}  // namespace symulator::tools::uid_generator
