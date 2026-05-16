// engine/include/engine/core/snapshot_service.hpp
// Serialises an EngineSnapshot to FlatBuffers wire format and splits it into
// ≤64 KB chunks suitable for SNAPSHOT_CHUNK frames (msg_type 0x31).
//
// All methods are static — SnapshotService carries no state.

#pragma once

#include "engine/core/engine_snapshot.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace engine::core
{

class SnapshotService
{
public:
    /// Maximum chunk size for SNAPSHOT_CHUNK frames (bytes).
    static constexpr std::size_t DEFAULT_CHUNK_SIZE = 64 * 1024;

    /// Serialise an EngineSnapshot to a FlatBuffers binary buffer.
    static std::vector<uint8_t> serialize(const EngineSnapshot& snap);

    /// Split `binary` into chunks of at most `chunk_size` bytes.
    /// Always returns at least one element (possibly empty when binary is empty).
    /// The consumer reassembles chunks in order before parsing the FlatBuffer.
    static std::vector<std::vector<uint8_t>> chunk(std::span<const uint8_t> binary,
                                                   std::size_t chunk_size = DEFAULT_CHUNK_SIZE);
};

}  // namespace engine::core
