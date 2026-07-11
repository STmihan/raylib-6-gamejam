#include "view/prefab/heal_wave.h"

#include <cmath>

#include "raylib.h"

#include "data/economy/economy.h"
#include "data/sim/sim_config.h"
#include "data/space/hex.h"
#include "data/time/time_config.h"
#include "logic/state/game_state.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    constexpr float TwoPi = 6.2831853f;
    constexpr float WorldPerTile = 100.0f * data::RenderScale;
}

void HealWaveView::Draw(const logic::GameState& state, float alpha) const
{
    BeginBlendMode(BLEND_ALPHA);
    for (int i = 0; i < data::MaxHealPulses; i++)
    {
        const logic::HealPulse& p = state.healPulses[i];
        if (!p.active) continue;

        float elapsed = (static_cast<float>(state.tick - p.startTick) + alpha) * static_cast<float>(data::TickDelta);
        float progress = elapsed / data::HealWaveSeconds;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) continue;

        float radius = static_cast<float>(p.radius) * WorldPerTile * progress;
        float thickness = 0.16f;
        auto a = static_cast<unsigned char>(190.0f * (1.0f - progress));
        Color color = {90, 255, 150, a};
        Vector3 center = LogicToWorld(p.center, 0.09f);

        float inner = radius - thickness;
        if (inner < 0.0f) inner = 0.0f;
        float outer = radius + thickness;

        const int seg = 44;
        for (int s = 0; s < seg; s++)
        {
            float t0 = TwoPi * static_cast<float>(s) / static_cast<float>(seg);
            float t1 = TwoPi * static_cast<float>(s + 1) / static_cast<float>(seg);
            Vector3 i0 = {center.x + std::cos(t0) * inner, center.y, center.z + std::sin(t0) * inner};
            Vector3 o0 = {center.x + std::cos(t0) * outer, center.y, center.z + std::sin(t0) * outer};
            Vector3 i1 = {center.x + std::cos(t1) * inner, center.y, center.z + std::sin(t1) * inner};
            Vector3 o1 = {center.x + std::cos(t1) * outer, center.y, center.z + std::sin(t1) * outer};
            DrawTriangle3D(i0, o0, o1, color);
            DrawTriangle3D(i0, o1, i1, color);
            DrawTriangle3D(o1, o0, i0, color);
            DrawTriangle3D(i1, o1, i0, color);
        }
    }
    EndBlendMode();
}
}
