#include "engine/timetable/timetable_catalog_loader.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace engine::timetable {

TimetablePointType parse_point_type(const std::string& type_str) {
    if (type_str == "STATION") return TimetablePointType::STATION;
    if (type_str == "PASSENGER_STOP") return TimetablePointType::PASSENGER_STOP;
    if (type_str == "JUNCTION_POST") return TimetablePointType::JUNCTION_POST;
    if (type_str == "SIDING_POST") return TimetablePointType::SIDING_POST;
    if (type_str == "LOADING_POINT") return TimetablePointType::LOADING_POINT;
    if (type_str == "PASSING_LOOP") return TimetablePointType::PASSING_LOOP;
    if (type_str == "BORDER_POINT") return TimetablePointType::BORDER_POINT;
    if (type_str == "TECHNICAL_POINT") return TimetablePointType::TECHNICAL_POINT;
    return TimetablePointType::OTHER;
}

TimetableCatalog TimetableCatalogLoader::load(const std::filesystem::path& data_root, std::error_code& ec) {
    TimetableCatalog catalog;
    ec.clear();

    auto points_dir = data_root / "data" / "timetable_points";
    if (std::filesystem::exists(points_dir)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(points_dir, ec)) {
            if (ec) return catalog;
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                std::ifstream f(entry.path());
                if (!f.is_open()) continue;

                nlohmann::json j;
                try {
                    f >> j;
                    TimetablePoint pt;
                    pt.uid.value = j.value("uid", 0ULL);
                    pt.name = j.value("name", "");
                    
                    std::string pt_type = j.value("point_type", "OTHER");
                    pt.point_type = parse_point_type(pt_type);

                    if (j.contains("short_name")) pt.short_name = j["short_name"].get<std::string>();
                    if (j.contains("abbreviation")) pt.abbreviation = j["abbreviation"].get<std::string>();
                    
                    if (j.contains("aliases")) {
                        for (const auto& a : j["aliases"]) pt.aliases.push_back(a.get<std::string>());
                    }
                    if (j.contains("external_ids")) {
                        for (auto it = j["external_ids"].begin(); it != j["external_ids"].end(); ++it) {
                            pt.external_ids[it.key()] = it.value().get<std::string>();
                        }
                    }
                    if (j.contains("railway_lines")) {
                        for (const auto& rl : j["railway_lines"]) pt.railway_lines.push_back(rl.get<std::string>());
                    }
                    if (j.contains("kilometer")) {
                        pt.kilometer = j["kilometer"].get<float>();
                    }

                    catalog.add_point(std::move(pt));
                } catch (const std::exception& e) {
                    std::cerr << "Failed to parse timetable point " << entry.path() << ": " << e.what() << "\n";
                }
            }
        }
    }

    auto conn_dir = data_root / "data" / "timetable_connections";
    if (std::filesystem::exists(conn_dir)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(conn_dir, ec)) {
            if (ec) return catalog;
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                std::ifstream f(entry.path());
                if (!f.is_open()) continue;

                nlohmann::json j;
                try {
                    f >> j;
                    TimetableConnection conn;
                    conn.uid.value = j.value("uid", 0ULL);
                    conn.from_uid.value = j.value("from_uid", 0ULL);
                    conn.to_uid.value = j.value("to_uid", 0ULL);
                    
                    if (j.contains("lines")) {
                        for (const auto& l : j["lines"]) conn.lines.push_back(l.get<std::string>());
                    }

                    catalog.add_connection(std::move(conn));
                } catch (const std::exception& e) {
                    std::cerr << "Failed to parse timetable connection " << entry.path() << ": " << e.what() << "\n";
                }
            }
        }
    }

    return catalog;
}

} // namespace engine::timetable
