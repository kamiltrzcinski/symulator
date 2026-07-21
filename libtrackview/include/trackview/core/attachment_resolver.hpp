#pragma once

#include "layout.hpp"

namespace trackview
{

class IAttachmentResolver
{
public:
    virtual ~IAttachmentResolver() = default;
    [[nodiscard]] virtual Point resolve(const Path& path,
                                        const TrackAttachment& attachment) const = 0;
};

class PathAttachmentResolver final : public IAttachmentResolver
{
public:
    [[nodiscard]] Point resolve(const Path& path,
                                const TrackAttachment& attachment) const override;
};

}  // namespace trackview
