// server/src/pip_writer.cpp

#include "server/pip_writer.hpp"

#include "engine/core/types.hpp"

#include <nlohmann/json.hpp>

namespace server
{

using engine::core::EntrySide;
using engine::core::PipEvent;
using engine::core::TrackOccupancy;

PipWriter::PipWriter(IDbWriter& db, std::string session_id)
    : db_{db}, session_id_{std::move(session_id)}
{
}

void PipWriter::on_pip_events(const std::vector<PipEvent>& events)
{
    for (const auto& ev : events)
    {
        std::string trains_json;

        if (ev.occupancy == TrackOccupancy::OCCUPIED && ev.slot.has_value())
        {
            const auto& slot = *ev.slot;
            nlohmann::json arr = nlohmann::json::array();
            nlohmann::json obj;
            obj["number"] = slot.number;
            obj["has_extra_info"] = slot.has_extra_info;
            obj["manually_placed"] = slot.manually_placed;
            obj["entry_side"] = (slot.entry_side == EntrySide::LEFT) ? "LEFT" : "RIGHT";
            arr.push_back(std::move(obj));
            trains_json = arr.dump();
        }
        else
        {
            trains_json = "[]";
        }

        db_.upsert_pip_track_state(session_id_, ev.section_uid.value, trains_json);
    }
}

}  // namespace server
