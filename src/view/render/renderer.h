#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include <functional>

#include "raylib.h"

#include "logic/state/game_state.h"
#include "logic/world/map.h"
#include "view/effect/water.h"
#include "view/prefab/model_registry.h"
#include "view/effect/water_params.h"
#include "view/render/shadow_params.h"
#include "view/render/cavity_params.h"

namespace view
{
class Renderer
{
public:
    void Init();
    void Shutdown();
    void Draw(const logic::GameState& previous, const logic::GameState& current, float alpha, Camera3D camera,
              const logic::Map& map, const std::function<void()>& overlay);

private:
    ModelRegistry models_;
    WaterEffect water_;
    ShadowParams shadowParams_;
    CavityParams cavityParams_;
    Shader toonShader_{};
    Shader geomShader_{};
    Shader shadowShader_{};
    Shader outlineShader_{};
    RenderTexture2D normalDepthTarget_{};
    RenderTexture2D colorTarget_{};
    RenderTexture2D shadowTarget_{};
    Camera3D lightCamera_{};
    Matrix lightProj_{};
    Vector3 lightTarget_{};
    float lightDist_ = 1.0f;
    int outlineNormalDepthLoc_ = 0;
    int screenRightLoc_ = 0;
    int screenUpLoc_ = 0;
    int cavityRadiusLoc_ = 0;
    int cavityValleyLoc_ = 0;
    int cavityRidgeLoc_ = 0;
    int shadowMapLoc_ = 0;
    int lightViewProjLoc_ = 0;
    int sunDirLoc_ = 0;
    int ambientLoc_ = 0;
    int bandsLoc_ = 0;
    int shadowStrengthLoc_ = 0;
    int softnessLoc_ = 0;
    int biasSlopeLoc_ = 0;
    int biasConstantLoc_ = 0;
    bool debugShadow_ = false;
};
}

#endif
