#include <trackview/core/attachment_resolver.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace trackview
{

Point PathAttachmentResolver::resolve(const Path& path,
                                      const TrackAttachment& attachment) const
{
    if (path.size() < 2)
        throw std::invalid_argument("attachment_resolver: path needs at least two points");
    if (!std::isfinite(attachment.offset) || attachment.offset < 0.0 ||
        attachment.offset > 1.0 || !std::isfinite(attachment.lateral))
        throw std::invalid_argument("attachment_resolver: invalid attachment values");

    std::vector<double> segment_lengths;
    segment_lengths.reserve(path.size() - 1);
    double total_length = 0.0;
    for (std::size_t index = 1; index < path.size(); ++index)
    {
        const double length = std::hypot(path[index].x - path[index - 1].x,
                                         path[index].y - path[index - 1].y);
        segment_lengths.push_back(length);
        total_length += length;
    }
    if (total_length <= 0.0)
        throw std::invalid_argument("attachment_resolver: path has zero length");

    const double fraction = attachment.reference_side == TrackSide::A
                                ? attachment.offset
                                : 1.0 - attachment.offset;
    double remaining = fraction * total_length;
    for (std::size_t index = 0; index < segment_lengths.size(); ++index)
    {
        const double length = segment_lengths[index];
        if (remaining <= length || index + 1 == segment_lengths.size())
        {
            if (length <= 0.0)
                continue;
            const auto& from = path[index];
            const auto& to = path[index + 1];
            const double ratio = std::clamp(remaining / length, 0.0, 1.0);
            const double dx = to.x - from.x;
            const double dy = to.y - from.y;
            return {from.x + ratio * dx - attachment.lateral * dy / length,
                    from.y + ratio * dy + attachment.lateral * dx / length};
        }
        remaining -= length;
    }
    throw std::logic_error("attachment_resolver: could not resolve path position");
}

}  // namespace trackview
