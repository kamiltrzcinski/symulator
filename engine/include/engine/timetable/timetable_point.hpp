#pragma once

#include "engine/core/types.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace engine::timetable {

enum class TimetablePointType : std::uint8_t {
    STATION,
    PASSENGER_STOP,
    JUNCTION_POST,
    SIDING_POST,
    LOADING_POINT,
    PASSING_LOOP,
    BORDER_POINT,
    TECHNICAL_POINT,
    OTHER
};

struct TimetablePoint {
    core::UID uid;
    std::string name;
    TimetablePointType point_type = TimetablePointType::OTHER;
    
    std::optional<std::string> short_name;
    std::optional<std::string> abbreviation;
    std::vector<std::string> aliases;
    std::unordered_map<std::string, std::string> external_ids;
    std::vector<std::string> railway_lines;
    std::optional<float> kilometer;
};

} // namespace engine::timetable
