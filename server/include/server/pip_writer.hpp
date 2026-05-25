// server/include/server/pip_writer.hpp
//
// PipWriter — consumes PipEvents produced by the ENGINE and writes them
// to pip.track_state via IDbWriter::upsert_pip_track_state.
//
// Called on the ENGINE thread from the pip_cb callback.
// Thread safety: single-threaded — do not share across threads without
// external synchronisation.

#pragma once

#include "server/db_writer.hpp"

#include "engine/core/types.hpp"

#include <string>
#include <vector>

namespace server
{

class PipWriter
{
public:
    /// @param db       Reference to the database writer (must outlive PipWriter).
    /// @param session_id  UUID string returned by IDbWriter::init_session.
    PipWriter(IDbWriter& db, std::string session_id);

    /// Process a batch of PipEvents and UPSERT pip.track_state for each one.
    void on_pip_events(const std::vector<engine::core::PipEvent>& events);

private:
    IDbWriter& db_;
    std::string session_id_;
};

}  // namespace server
