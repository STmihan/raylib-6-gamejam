#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include <functional>

#include "raylib.h"

#include "logic/game_state.h"
#include "logic/map.h"
#include "view/model_registry.h"
#include "view/water_params.h"

namespace view
{
class Renderer
{
public:
    void Init();
    void Shutdown();
    void Draw(const logic::GameState& previous, const logic::GameState& current, float alpha, Camera3D camera,
              const logic::Map& map, const std::function<void()>& overlay);

    WaterParams& WaterParamsRef() { return waterParams_; }

private:
    ModelRegistry models_;
    Shader toonShader_{};
    Shader geomShader_{};
    Shader shadowShader_{};
    Shader outlineShader_{};
    Shader waterShader_{};
    RenderTexture2D normalDepthTarget_{};
    RenderTexture2D colorTarget_{};
    RenderTexture2D shadowTarget_{};
    Texture2D sdfTexture_{};
    Camera3D lightCamera_{};
    Vector2 sdfOrigin_{};
    float sdfWorldSize_ = 1.0f;
    int outlineNormalDepthLoc_ = 0;
    int shadowMapLoc_ = 0;
    int lightViewProjLoc_ = 0;
    int waterTimeLoc_ = 0;
    int waterSdfMapLoc_ = 0;
    WaterParams waterParams_{};
};
}

#endif
