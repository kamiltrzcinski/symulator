#include "services/clipboard_service.hpp"

#include <QClipboard>
#include <QGuiApplication>
#include <QString>

#include <stdexcept>

namespace symulator::tools
{

void UidClipboardService::copyUid(UID uid) const
{
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard == nullptr)
    {
        throw std::runtime_error("System clipboard is not available");
    }

    clipboard->setText(QString::number(static_cast<qulonglong>(uid.value)));
}

}  // namespace symulator::tools
