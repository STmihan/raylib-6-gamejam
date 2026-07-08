#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include <functional>

#include "raylib.h"

#include "data/render/cavity_params.h"
#include "data/render/shadow_params.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"
#include "view/effect/outline_effect.h"
#include "view/effect/shadow_effect.h"
#include "view/effect/toon_effect.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/registries/shader_registry.h"
#include "view/prefab/water.h"

namespace view
{
class Renderer
{
public:
    void Init();
    void Shutdown();
    void Draw(const logic::GameState& previous, const logic::GameState& current, float alpha, Camera3D camera,
              const logic::Map& map, const std::function<void()>& overlay);

    WaterEffect& Water() { return water_; }
    data::ShadowParams& ShadowParamsRef() { return shadowParams_; }
    data::CavityParams& CavityParamsRef() { return cavityParams_; }

private:
    ShaderRegistry shaders_;
    ModelRegistry models_;
    ShadowEffect shadow_;
    ToonEffect toon_;
    OutlineEffect outline_;
    WaterEffect water_;
    RenderTexture2D colorTarget_{};
    data::ShadowParams shadowParams_;
    data::CavityParams cavityParams_;
};
}

#endif
