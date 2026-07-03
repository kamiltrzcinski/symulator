// server/src/terminal/trains_command.cpp

#include "server/terminal/trains_command.hpp"

#include <iomanip>
#include <sstream>

namespace server::terminal
{

TrainsCommand::TrainsCommand(const engine::core::AtomicSnapshot& snapshot) : snapshot_(snapshot) {}

std::string TrainsCommand::execute(const std::vector<std::string>& args)
{
    (void)args;

    const auto snap = snapshot_.load();
    if (!snap)
        return "world snapshot not available yet";
    if (snap->trains.empty())
        return "no active trains";

    std::ostringstream out;
    out << "active trains (" << snap->trains.size() << "):";
    for (const auto& train : snap->trains)
    {
        out << "\n  uid " << train.uid.value << "  section " << train.section_uid.value << "  "
            << std::fixed << std::setprecision(1) << train.speed_kmh << " km/h  "
            << train.total_axles << " axles";
    }
    return out.str();
}

}  // namespace server::terminal
