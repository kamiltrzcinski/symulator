#pragma once

#include "engine/core/types.hpp"
#include <vector>
#include <string>

namespace engine::timetable {

struct TimetableConnection {
    core::UID uid;
    core::UID from_uid;
    core::UID to_uid;
    std::vector<std::string> lines;
};

} // namespace engine::timetable
