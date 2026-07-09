#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include <functional>

#include "raylib.h"

#include "data/render/cavity_params.h"
#include "data/render/shadow_params.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"
#include "debug/air_test_scene.h"
#include "debug/preview_scene.h"
#include "debug/projectile_test_scene.h"
#include "view/anim/anim_controller.h"
#include "view/effect/outline_effect.h"
#include "view/effect/shadow_effect.h"
#include "view/effect/toon_effect.h"
#include "view/prefab/hex_grid.h"
#include "view/prefab/hp_bar.h"
#include "view/prefab/registries/muzzle_registry.h"
#include "view/prefab/plane_orbit.h"
#include "view/prefab/projectile_view.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/registries/shader_registry.h"
#include "view/prefab/unit_view.h"
#include "view/prefab/water.h"

namespace view
{
class Renderer
{
public:
    void Init();
    void Shutdown();
    void Draw(const logic::GameState& previous, const logic::GameState& current, float alpha, float animTime,
              Camera3D camera, const logic::Map& map, const std::function<void()>& overlay);

    WaterEffect& Water() { return water_; }
    HexGrid& Grid() { return hexGrid_; }
    OutlineEffect& Outline() { return outline_; }
    debug::PreviewScene& Preview() { return preview_; }
    debug::AirTestScene& AirTest() { return airTest_; }
    debug::ProjectileTestScene& ProjTest() { return projTest_; }
    UnitView& Units() { return units_; }
    ProjectileView& Projectiles() { return projectiles_; }
    PlaneOrbitParams& OrbitParams() { return orbitParams_; }
    data::ShadowParams& ShadowParamsRef() { return shadowParams_; }
    data::ShadowParams& UnitShadowParamsRef() { return unitShadowParams_; }
    data::CavityParams& CavityParamsRef() { return cavityParams_; }

private:
    void DrawPreview(const std::function<void()>& overlay);
    void DrawAirTest(const std::function<void()>& overlay);
    void DrawProjectileTest(const std::function<void()>& overlay);

    ShaderRegistry shaders_;
    ModelRegistry models_;
    ShadowEffect shadow_;
    ToonEffect toon_;
    OutlineEffect outline_;
    WaterEffect water_;
    UnitView units_;
    HexGrid hexGrid_;
    HpBarView hpBars_;
    AnimController anim_;
    MuzzleRegistry muzzles_;
    ProjectileView projectiles_;
    PlaneOrbitParams orbitParams_;
    debug::PreviewScene preview_;
    debug::AirTestScene airTest_;
    debug::ProjectileTestScene projTest_;
    bool hexGridLoaded_ = false;
    RenderTexture2D colorTarget_{};
    data::ShadowParams shadowParams_;
    data::ShadowParams unitShadowParams_;
    data::CavityParams cavityParams_;
};
}

#endif
