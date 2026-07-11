#ifndef VIEW_RENDERER_H
#define VIEW_RENDERER_H

#include <array>
#include <functional>

#include "raylib.h"

#include "data/render/cavity_params.h"
#include "data/render/shadow_params.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"
#include "debug/preview_scene.h"
#include "debug/projectile_test_scene.h"
#include "view/anim/anim_controller.h"
#include "view/prefab/deploy_ring.h"
#include "view/effect/outline_effect.h"
#include "view/effect/shadow_effect.h"
#include "view/effect/toon_effect.h"
#include "view/prefab/hex_grid.h"
#include "view/prefab/control_overlay.h"
#include "view/prefab/damage_numbers.h"
#include "view/prefab/heal_wave.h"
#include "view/prefab/hp_bar.h"
#include "view/prefab/registries/muzzle_registry.h"
#include "view/prefab/plane_orbit.h"
#include "view/prefab/projectile_view.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/registries/shader_registry.h"
#include "view/prefab/registries/texture_registry.h"
#include "view/prefab/unit_view.h"
#include "view/prefab/water.h"
#include "view/prefab/ui/card_view.h"
#include "view/prefab/ui/hand_view.h"
#include "view/prefab/ui/match_timer.h"
#include "view/prefab/ui/resource_bar.h"
#include "view/prefab/ui/ui_context.h"
#include "view/prefab/ui/ui_widgets.h"

namespace view
{
class Renderer
{
public:
    void Init(const logic::Map& map);
    void Shutdown();
    void Draw(const logic::GameState& previous, const logic::GameState& current, float alpha, float animTime,
              Camera3D camera, const logic::Map& map, const std::function<void()>& overlay);

    WaterEffect& Water() { return water_; }
    HexGrid& Grid() { return hexGrid_; }
    OutlineEffect& Outline() { return outline_; }
    debug::PreviewScene& Preview() { return preview_; }
    debug::ProjectileTestScene& ProjTest() { return projTest_; }
    UnitView& Units() { return units_; }
    ProjectileView& Projectiles() { return projectiles_; }
    PlaneOrbitParams& OrbitParams() { return orbitParams_; }
    data::ShadowParams& ShadowParamsRef() { return shadowParams_; }
    data::ShadowParams& UnitShadowParamsRef() { return unitShadowParams_; }
    bool& HudHiddenRef() { return hudHidden_; }
    bool& DragZoneRef() { return dragZone_; }
    int& HudResourceHighlightRef() { return resourceHighlight_; }
    data::CrystalStyle& CrystalStyleRef() { return crystalStyle_; }
    ui::HandView& Hand() { return hand_; }
    ui::UiContext& Ui() { return ui_; }
    ControlOverlayView& ControlOverlay() { return controlOverlay_; }
    data::CavityParams& CavityParamsRef() { return cavityParams_; }

private:
    struct ScenePasses
    {
        std::function<void()> shadow;
        std::function<void()> geom;
        std::function<void()> mask;
        std::function<void()> color;
        std::function<void()> composite2D;
    };

    void DrawPreview(const std::function<void()>& overlay);
    void DrawProjectileTest(const logic::Map& map, const std::function<void()>& overlay);
    void RenderPasses(Camera3D camera, const ScenePasses& passes, const std::function<void()>& overlay);
    void DrawDeployOverlay(Camera3D camera, const logic::Map& map, float time, bool affordable);
    void ConfigureBlobShadow();
    void UseEnvShadow();
    void UseUnitShadow();

    ShaderRegistry shaders_;
    TextureRegistry textures_;
    ModelRegistry models_;
    ShadowEffect shadow_;
    ToonEffect toon_;
    OutlineEffect outline_;
    WaterEffect water_;
    UnitView units_;
    HexGrid hexGrid_;
    HpBarView hpBars_;
    HealWaveView healWaves_;
    DamageNumbers damageNumbers_;
    ControlOverlayView controlOverlay_;
    DeployRingView deployRings_;
    AnimController anim_;
    MuzzleRegistry muzzles_;
    ProjectileView projectiles_;
    PlaneOrbitParams orbitParams_;
    debug::PreviewScene preview_;
    debug::ProjectileTestScene projTest_;
    ui::UiContext ui_;
    ui::HandView hand_;
    bool hudHidden_ = false;
    bool dragZone_ = false;
    int resourceHighlight_ = 0;
    data::CrystalStyle crystalStyle_;
    bool hexGridLoaded_ = false;
    std::array<bool, logic::MapTileCount> occluded_{};
    std::array<bool, logic::MapTileCount> wallAlive_{};
    RenderTexture2D colorTarget_{};
    RenderTexture2D cardTarget_{};
    data::ShadowParams shadowParams_;
    data::ShadowParams unitShadowParams_;
    data::CavityParams cavityParams_;
};
}

#endif
