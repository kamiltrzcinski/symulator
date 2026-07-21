#pragma once

#include "attachment_resolver.hpp"
#include "render_model.hpp"

namespace trackview
{

class IRenderModelBuilder
{
public:
    virtual ~IRenderModelBuilder() = default;
    [[nodiscard]] virtual RenderModel build(const ITrackRuntimeState& state,
                                            const TrackLayout& layout) const = 0;
};

class RenderModelBuilder final : public IRenderModelBuilder
{
public:
    explicit RenderModelBuilder(const IAttachmentResolver& attachment_resolver)
        : attachment_resolver_(attachment_resolver)
    {
    }

    [[nodiscard]] RenderModel build(const ITrackRuntimeState& state,
                                    const TrackLayout& layout) const override;

private:
    const IAttachmentResolver& attachment_resolver_;
};

}  // namespace trackview
