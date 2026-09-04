#pragma once

#include "engine/core/types.hpp"
#include "engine/timetable/timetable_point.hpp"
#include "engine/timetable/timetable_connection.hpp"

#include <unordered_map>
#include <vector>
#include <optional>

namespace engine::timetable {

class TimetableCatalog {
public:
    void add_point(TimetablePoint point) {
        points_[point.uid] = std::move(point);
    }

    void add_connection(TimetableConnection conn) {
        connections_[conn.uid] = conn;
        adjacency_list_[conn.from_uid].push_back(conn.to_uid);
    }

    [[nodiscard]] bool contains_point(core::UID uid) const {
        return points_.contains(uid);
    }

    [[nodiscard]] std::optional<TimetablePoint> find_point(core::UID uid) const {
        if (auto it = points_.find(uid); it != points_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
    // Finds a point by exact name (for legacy migration/fallback)
    [[nodiscard]] std::optional<TimetablePoint> find_point_by_name(const std::string& name) const {
        for (const auto& [uid, pt] : points_) {
            if (pt.name == name) {
                return pt;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] bool are_connected(core::UID from_uid, core::UID to_uid) const {
        if (auto it = adjacency_list_.find(from_uid); it != adjacency_list_.end()) {
            for (core::UID neighbor : it->second) {
                if (neighbor == to_uid) {
                    return true;
                }
            }
        }
        return false;
    }

    [[nodiscard]] bool has_connections() const {
        return !connections_.empty();
    }

    [[nodiscard]] const std::unordered_map<core::UID, TimetablePoint>& get_all_points() const {
        return points_;
    }

private:
    std::unordered_map<core::UID, TimetablePoint> points_;
    std::unordered_map<core::UID, TimetableConnection> connections_;
    std::unordered_map<core::UID, std::vector<core::UID>> adjacency_list_;
};

} // namespace engine::timetable
