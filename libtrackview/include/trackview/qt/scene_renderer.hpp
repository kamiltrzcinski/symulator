#pragma once

#include "track_theme.hpp"

#include <trackview/core/render_model.hpp>

#include <QGraphicsScene>
#include <QImage>

namespace trackview
{

inline constexpr int kInfrastructureIdDataKey = 0;

class ISceneRenderer
{
public:
    virtual ~ISceneRenderer() = default;
    virtual void render(const RenderModel& model, QGraphicsScene& scene) const = 0;
};

class IImageRenderer
{
public:
    virtual ~IImageRenderer() = default;
    [[nodiscard]] virtual QImage render(const RenderModel& model,
                                        double scale) const = 0;
};

class QtSceneRenderer final : public ISceneRenderer
{
public:
    explicit QtSceneRenderer(const ITrackTheme& theme) : theme_(theme) {}

    void render(const RenderModel& model, QGraphicsScene& scene) const override;
    void render(const RenderModel& model, QGraphicsScene& scene, bool interactive) const;

private:
    const ITrackTheme& theme_;
};

class QtImageRenderer final : public IImageRenderer
{
public:
    explicit QtImageRenderer(const ISceneRenderer& scene_renderer)
        : scene_renderer_(scene_renderer)
    {
    }

    [[nodiscard]] QImage render(const RenderModel& model,
                                double scale) const override;

private:
    const ISceneRenderer& scene_renderer_;
};

}  // namespace trackview
