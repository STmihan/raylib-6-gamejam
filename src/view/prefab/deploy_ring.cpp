#include "view/prefab/deploy_ring.h"

#include "data/economy/economy.h"
#include "data/sim/sim_config.h"
#include "view/space/interpolator.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    constexpr float RingHeight = 1.0f;
    constexpr float RingSize = 34.0f;
}

void DeployRingView::Load(Shader ring, Texture2D white)
{
    ring_ = ring;
    white_ = white;
    locProgress_ = GetShaderLocation(ring_, "progress");

    float innerR = 0.34f;
    float fg[4] = {0.90f, 0.94f, 1.0f, 0.92f};
    float bg[4] = {0.06f, 0.08f, 0.05f, 0.55f};
    SetShaderValue(ring_, GetShaderLocation(ring_, "innerR"), &innerR, SHADER_UNIFORM_FLOAT);
    SetShaderValue(ring_, GetShaderLocation(ring_, "fgColor"), fg, SHADER_UNIFORM_VEC4);
    SetShaderValue(ring_, GetShaderLocation(ring_, "bgColor"), bg, SHADER_UNIFORM_VEC4);
    loaded_ = true;
}

void DeployRingView::Draw(Camera3D camera, const logic::GameState& previous, const logic::GameState& current,
                          float alpha) const
{
    if (!loaded_) return;
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& e = current.entities[i];
        if (!e.active || e.kind != logic::EntityKind::Unit) continue;

        float prevTimer = previous.entities[i].active ? previous.entities[i].deployTimer : e.deployTimer;
        float timer = prevTimer * (1.0f - alpha) + e.deployTimer * alpha;
        if (timer <= 0.001f) continue;

        float progress = 1.0f - timer / data::DeployFreezeSeconds();
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;

        data::Vec2 logic = InterpolatedPosition(previous.entities[i], e, alpha);
        Vector2 screen = GetWorldToScreen(LogicToWorld(logic, RingHeight), camera);
        Rectangle dst = {screen.x - RingSize * 0.5f, screen.y - RingSize * 0.5f, RingSize, RingSize};

        SetShaderValue(ring_, locProgress_, &progress, SHADER_UNIFORM_FLOAT);
        BeginShaderMode(ring_);
        DrawTexturePro(white_, Rectangle{0.0f, 0.0f, 1.0f, 1.0f}, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        EndShaderMode();
    }
}
}
