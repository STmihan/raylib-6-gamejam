#ifndef VIEW_EFFECT_OUTLINE_EFFECT_H
#define VIEW_EFFECT_OUTLINE_EFFECT_H

#include <functional>

#include "raylib.h"

#include "data/render/cavity_params.h"

namespace view
{
class ModelRegistry;

class OutlineEffect
{
public:
    void Init(Shader geomShader, Shader outlineShader, Shader maskShader);
    void Shutdown();

    void RenderNormalDepth(ModelRegistry& models, Camera3D camera, const std::function<void()>& drawScene);
    void RenderUnitMask(ModelRegistry& models, Camera3D camera, const std::function<void()>& drawExtra);
    void Composite(RenderTexture2D colorTarget, const data::CavityParams& cavity, Camera3D camera);

    float& UnitOutlineScaleRef() { return unitOutlineScale_; }
    float& UnitCavityScaleRef() { return unitCavityScale_; }

private:
    Shader geomShader_{};
    Shader outlineShader_{};
    Shader maskShader_{};
    RenderTexture2D normalDepthTarget_{};
    RenderTexture2D maskTarget_{};
    int normalDepthLoc_ = 0;
    int unitMaskLoc_ = 0;
    int screenRightLoc_ = 0;
    int screenUpLoc_ = 0;
    int cavityRadiusLoc_ = 0;
    int cavityValleyLoc_ = 0;
    int cavityRidgeLoc_ = 0;
    int unitOutlineScaleLoc_ = 0;
    int unitCavityScaleLoc_ = 0;
    float unitOutlineScale_ = 0.2f;
    float unitCavityScale_ = 1.0f;
};
}

#endif
