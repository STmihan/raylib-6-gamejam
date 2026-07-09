#ifndef VIEW_EFFECT_SHADOW_EFFECT_H
#define VIEW_EFFECT_SHADOW_EFFECT_H

#include <functional>

#include "raylib.h"

namespace view
{
class ModelRegistry;

class ShadowEffect
{
public:
    void Init(Shader shader);
    void Shutdown();

    void RenderMap(ModelRegistry& models, Vector3 sunDir, const std::function<void()>& drawScene);
    void DrawDebugPreview() const;

    Matrix LightViewProj() const { return lightViewProj_; }
    unsigned int DepthTextureId() const { return target_.depth.id; }
    int Slot() const { return ShadowSlot; }

private:
    static constexpr int ShadowSlot = 10;
    Shader shader_{};
    RenderTexture2D target_{};
    Camera3D lightCamera_{};
    Matrix lightProj_{};
    Matrix lightViewProj_{};
    Vector3 lightTarget_{};
    float lightDist_ = 1.0f;
};
}

#endif
