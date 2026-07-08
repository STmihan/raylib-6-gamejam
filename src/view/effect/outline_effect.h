#ifndef VIEW_EFFECT_OUTLINE_EFFECT_H
#define VIEW_EFFECT_OUTLINE_EFFECT_H

#include "raylib.h"

#include "data/render/cavity_params.h"

namespace logic { struct Map; }

namespace view
{
class ModelRegistry;
class Scene;

class OutlineEffect
{
public:
    void Init(Shader geomShader, Shader outlineShader);
    void Shutdown();

    void RenderNormalDepth(ModelRegistry& models, const Scene& scene, const logic::Map& map, Camera3D camera);
    void Composite(RenderTexture2D colorTarget, const data::CavityParams& cavity, Camera3D camera);

private:
    Shader geomShader_{};
    Shader outlineShader_{};
    RenderTexture2D normalDepthTarget_{};
    int normalDepthLoc_ = 0;
    int screenRightLoc_ = 0;
    int screenUpLoc_ = 0;
    int cavityRadiusLoc_ = 0;
    int cavityValleyLoc_ = 0;
    int cavityRidgeLoc_ = 0;
};
}

#endif
