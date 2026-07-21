#pragma once

#include "layout.hpp"

#include <filesystem>

namespace trackview
{

class ILayoutReader
{
public:
    virtual ~ILayoutReader() = default;
    [[nodiscard]] virtual TrackLayout read(const std::filesystem::path& path) const = 0;
};

class JsonLayoutReader final : public ILayoutReader
{
public:
    [[nodiscard]] TrackLayout read(const std::filesystem::path& path) const override;
};

}  // namespace trackview
