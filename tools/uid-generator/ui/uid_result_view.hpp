#pragma once

#include <QWidget>

#include "domain/uid_types.hpp"

class QLabel;
class QPushButton;

namespace symulator::tools
{
class UidClipboardService;
}

namespace symulator::tools::uid_generator
{

class UidResultView final : public QWidget
{
    Q_OBJECT

public:
    explicit UidResultView(const UidClipboardService& clipboard, QWidget* parent = nullptr);

    void showUid(UID uid);
    [[nodiscard]] UID displayedUid() const noexcept;

private slots:
    void copyUid();

private:
    const UidClipboardService& clipboard_;
    QLabel* decimal_value_ = nullptr;
    QLabel* hex_value_ = nullptr;
    QPushButton* copy_button_ = nullptr;
    UID uid_{};
};

}  // namespace symulator::tools::uid_generator
