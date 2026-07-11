#include "view/prefab/damage_numbers.h"

#include <cstdio>

#include "logic/state/game_state.h"
#include "view/prefab/ui/ui_widgets.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
constexpr float Lifetime = 0.85f;
constexpr float MergeWindow = 0.25f;
constexpr float RisePixels = 40.0f;
constexpr std::uint32_t InvalidId = 0xFFFFFFFFu;

void DrawBoldCentered(ui::UiContext& ui, const char* text, Vector2 center, float size, Color color)
{
    Vector2 extent = ui.Text().Measure(text, size);
    Vector2 pos = {center.x - extent.x * 0.5f, center.y - extent.y * 0.5f};
    float o = size * 0.08f;
    ui.Text().Draw(text, Vector2{pos.x - o, pos.y}, size, color);
    ui.Text().Draw(text, Vector2{pos.x + o, pos.y}, size, color);
    ui.Text().Draw(text, Vector2{pos.x, pos.y - o}, size, color);
    ui.Text().Draw(text, Vector2{pos.x, pos.y + o}, size, color);
    ui.Text().Draw(text, pos, size, color);
}
}

void DamageNumbers::Reset()
{
    for (Popup& p : pool_) p.active = false;
    for (int i = 0; i < data::MaxEntities; i++)
    {
        lastHp_[i] = 0;
        lastId_[i] = InvalidId;
        recent_[i] = -1;
    }
    for (int i = 0; i < data::MaxMisses; i++) missSeen_[i] = 0;
}

int DamageNumbers::FindSlot()
{
    int oldest = 0;
    float oldestAge = -1.0f;
    for (int i = 0; i < Max; i++)
    {
        if (!pool_[i].active) return i;
        if (pool_[i].age > oldestAge)
        {
            oldestAge = pool_[i].age;
            oldest = i;
        }
    }
    return oldest;
}

void DamageNumbers::Update(const logic::GameState& current, float dt)
{
    if (!init_ || current.tick < lastTick_)
    {
        Reset();
        init_ = true;
    }
    lastTick_ = current.tick;

    for (int i = 0; i < Max; i++)
    {
        Popup& p = pool_[i];
        if (!p.active) continue;
        p.age += dt;
        if (p.age >= Lifetime)
        {
            p.active = false;
            if (p.owner >= 0 && p.owner < data::MaxEntities && recent_[p.owner] == i)
                recent_[p.owner] = -1;
        }
    }

    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& e = current.entities[i];
        if (!e.active)
        {
            lastId_[i] = InvalidId;
            recent_[i] = -1;
            continue;
        }
        if (lastId_[i] != e.id)
        {
            lastId_[i] = e.id;
            lastHp_[i] = e.hp;
            recent_[i] = -1;
            continue;
        }

        int delta = e.hp - lastHp_[i];
        lastHp_[i] = e.hp;
        if (delta == 0) continue;

        bool heal = delta > 0;
        int magnitude = heal ? delta : -delta;

        int slot = recent_[i];
        bool merged = false;
        if (slot >= 0 && slot < Max)
        {
            Popup& r = pool_[slot];
            if (r.active && r.owner == i && r.heal == heal && r.age < MergeWindow)
            {
                r.amount += magnitude;
                r.age = 0.0f;
                r.origin = e.position;
                std::snprintf(r.text, sizeof(r.text), "%s%d", heal ? "+" : "-", r.amount);
                merged = true;
            }
        }

        if (!merged)
        {
            int idx = FindSlot();
            Popup& p = pool_[idx];
            p.active = true;
            p.owner = i;
            p.origin = e.position;
            p.amount = magnitude;
            p.heal = heal;
            p.miss = false;
            p.age = 0.0f;
            std::snprintf(p.text, sizeof(p.text), "%s%d", heal ? "+" : "-", magnitude);
            recent_[i] = idx;
        }
    }

    for (int i = 0; i < data::MaxMisses; i++)
    {
        const logic::MissMark& mark = current.misses[i];
        if (!mark.active)
        {
            missSeen_[i] = 0;
            continue;
        }
        if (missSeen_[i] == mark.startTick) continue;
        missSeen_[i] = mark.startTick;

        int idx = FindSlot();
        Popup& p = pool_[idx];
        p.active = true;
        p.owner = -1;
        p.origin = mark.position;
        p.amount = 0;
        p.heal = false;
        p.miss = true;
        p.age = 0.0f;
        std::snprintf(p.text, sizeof(p.text), "MISS");
    }
}

void DamageNumbers::Draw(ui::UiContext& ui, Camera3D camera) const
{
    const Color damage = {255, 90, 80, 255};
    const Color healColor = {90, 235, 130, 255};
    for (const Popup& p : pool_)
    {
        if (!p.active) continue;
        float t = p.age / Lifetime;
        Vector3 world = LogicToWorld(p.origin, 1.4f);
        Vector2 screen = GetWorldToScreen(world, camera);
        if (screen.x < -60.0f || screen.x > GetScreenWidth() + 60.0f ||
            screen.y < -40.0f || screen.y > GetScreenHeight() + 40.0f)
            continue;
        screen.y -= t * RisePixels;

        Color color = p.miss ? Color{240, 240, 240, 255} : (p.heal ? healColor : damage);
        if (t > 0.65f)
        {
            float fade = 1.0f - (t - 0.65f) / 0.35f;
            color.a = static_cast<unsigned char>(255.0f * (fade < 0.0f ? 0.0f : fade));
        }

        float size = p.miss ? 13.0f : 12.0f;
        DrawBoldCentered(ui, p.text, screen, size, color);
    }
}
}
