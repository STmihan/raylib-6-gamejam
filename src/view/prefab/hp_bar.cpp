#include "view/prefab/hp_bar.h"

#include "data/sim/sim_config.h"
#include "data/unit/unit.h"
#include "view/space/interpolator.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    constexpr float BarWidth = 42.0f;
    constexpr float BarHeight = 6.0f;
    constexpr float BarBorder = 1.0f;

    const Color BackColor = {18, 18, 24, 220};
    const Color BorderColor = {0, 0, 0, 230};
    const Color TopFill = {235, 85, 70, 255};
    const Color BottomFill = {80, 150, 235, 255};

    int MaxHpOf(const logic::Entity& entity)
    {
        switch (entity.kind)
        {
        case logic::EntityKind::Base: return data::BaseHp;
        case logic::EntityKind::Wall: return data::WallHp;
        default: return data::UnitStatsOf(entity.type).hp;
        }
    }

    float BarWorldHeight(const logic::Entity& entity)
    {
        switch (entity.kind)
        {
        case logic::EntityKind::Base: return 3.2f;
        case logic::EntityKind::Wall: return 1.2f;
        default: return entity.type == data::UnitType::Plane ? 2.4f : 1.4f;
        }
    }
}

void HpBarView::Load(Texture2D white)
{
    white_ = white;
    loaded_ = true;
}

void HpBarView::Bar(Vector2 topCenter, float fraction, Color fill) const
{
    fraction = fraction < 0.0f ? 0.0f : (fraction > 1.0f ? 1.0f : fraction);
    float left = topCenter.x - BarWidth * 0.5f;
    Rectangle source = {0.0f, 0.0f, 1.0f, 1.0f};

    Rectangle border = {left - BarBorder, topCenter.y - BarBorder, BarWidth + 2.0f * BarBorder,
                        BarHeight + 2.0f * BarBorder};
    DrawTexturePro(white_, source, border, Vector2{0.0f, 0.0f}, 0.0f, BorderColor);
    DrawTexturePro(white_, source, Rectangle{left, topCenter.y, BarWidth, BarHeight}, Vector2{0.0f, 0.0f}, 0.0f,
                   BackColor);
    DrawTexturePro(white_, source, Rectangle{left, topCenter.y, BarWidth * fraction, BarHeight},
                   Vector2{0.0f, 0.0f}, 0.0f, fill);
}

void HpBarView::DrawSingle(Camera3D camera, Vector3 worldTop, float fraction, Color fill) const
{
    if (!loaded_) return;
    Vector2 screen = GetWorldToScreen(worldTop, camera);
    Bar(screen, fraction, fill);
}

void HpBarView::DrawEntities(Camera3D camera, const logic::GameState& previous,
                             const logic::GameState& current, float alpha) const
{
    if (!loaded_) return;
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& entity = current.entities[i];
        if (!entity.active) continue;

        int maxHp = MaxHpOf(entity);
        if (entity.kind != logic::EntityKind::Unit && entity.hp >= maxHp) continue;

        data::Vec2 logic = InterpolatedPosition(previous.entities[i], entity, alpha);
        Vector3 worldTop = LogicToWorld(logic, BarWorldHeight(entity));
        Vector2 screen = GetWorldToScreen(worldTop, camera);

        float fraction = static_cast<float>(entity.hp) / static_cast<float>(maxHp);
        Color fill = entity.team == data::Team::Top ? TopFill : BottomFill;
        Bar(screen, fraction, fill);
    }
}
}
