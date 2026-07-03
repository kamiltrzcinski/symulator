// server/src/terminal/lookup.cpp

#include "server/terminal/lookup.hpp"

#include <charconv>

namespace server::terminal
{

std::optional<std::uint64_t> parse_uint(std::string_view text)
{
    std::uint64_t value = 0;
    const char* end = text.data() + text.size();
    auto [ptr, ec] = std::from_chars(text.data(), end, value);
    if (ec != std::errc{} || ptr != end)
        return std::nullopt;
    return value;
}

std::optional<engine::core::UID> resolve_consist_uid(const engine::core::FleetRegistry& fleet,
                                                     std::string_view arg)
{
    if (auto value = parse_uint(arg))
    {
        const engine::core::UID uid{*value};
        if (fleet.has_consist(uid))
            return uid;
        return std::nullopt;
    }

    for (const auto& [uid, consist] : fleet.all_consists())
    {
        if (consist.pid == arg)
            return uid;
    }
    return std::nullopt;
}

std::optional<engine::core::UID> resolve_boundary_uid(const engine::core::EngineSnapshot& snap,
                                                      std::string_view arg)
{
    if (auto value = parse_uint(arg))
    {
        const engine::core::UID uid{*value};
        if (snap.boundary_nodes.contains(uid))
            return uid;
        return std::nullopt;
    }

    for (const auto& [uid, node] : snap.boundary_nodes)
    {
        if (node.pid == arg)
            return uid;
    }
    return std::nullopt;
}

}  // namespace server::terminal
