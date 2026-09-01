#pragma once

#include "engine/timetable/timetable_catalog.hpp"
#include <filesystem>
#include <string>
#include <system_error>

namespace engine::timetable {

class TimetableCatalogLoader {
public:
    // Loads the catalog from a symulator-data root path
    static TimetableCatalog load(const std::filesystem::path& data_root, std::error_code& ec);
};

} // namespace engine::timetable
