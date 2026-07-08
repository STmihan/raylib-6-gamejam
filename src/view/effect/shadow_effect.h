#ifndef VIEW_EFFECT_SHADOW_EFFECT_H
#define VIEW_EFFECT_SHADOW_EFFECT_H

#include "raylib.h"

namespace logic { struct Map; }

namespace view
{
class ModelRegistry;
class Scene;

class ShadowEffect
{
public:
    void Init(Shader shader);
    void Shutdown();

    void RenderMap(ModelRegistry& models, const Scene& scene, const logic::Map& map, Vector3 sunDir);
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
