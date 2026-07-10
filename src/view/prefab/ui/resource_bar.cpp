#include "view/prefab/ui/resource_bar.h"

#include <cmath>

namespace view::ui
{
namespace
{
    const Color SlotDark = {15, 23, 16, 255};

    constexpr float Gap = 4.0f;
    constexpr float Pad = 2.0f;
    constexpr float Ring = 2.0f;

    Rectangle Inset(Rectangle r, float n)
    {
        return Rectangle{r.x + n, r.y + n, r.width - 2.0f * n, r.height - 2.0f * n};
    }
}

void DrawResourceBar(UiContext& ui, Rectangle rect, float resource, int cap, int highlight, float time,
                     const data::CrystalStyle& style)
{
    if (cap <= 0) return;
    float w = (rect.width - Gap * static_cast<float>(cap - 1)) / static_cast<float>(cap);
    float pulse = 0.5f + 0.5f * std::sin(time * 6.0f);
    int available = static_cast<int>(resource);

    for (int i = 0; i < cap; i++)
    {
        Rectangle slot = {rect.x + static_cast<float>(i) * (w + Gap), rect.y, w, rect.height};
        float fill = resource - static_cast<float>(i);
        fill = fill < 0.0f ? 0.0f : (fill > 1.0f ? 1.0f : fill);
        bool highlighted = i < highlight;
        bool has = i < available;

        if (highlighted && !has)
        {
            Color red = {232, 74, 74, static_cast<unsigned char>(110.0f + 130.0f * pulse)};
            ui.Theme().Panel(slot, red);
            ui.Theme().Panel(Inset(slot, Ring), SlotDark);
        }
        else
        {
            ui.Theme().Panel(slot, SlotDark);
        }

        if (fill > 0.0f)
        {
            Rectangle inner = Inset(slot, Pad + (highlighted && !has ? Ring : 0.0f));
            float ow = style.outlineWidth;
            float rightEdge = inner.x + inner.width * fill + (fill >= 1.0f ? ow : 0.0f);
            int sx = static_cast<int>(std::floor(inner.x - ow));
            int sy = static_cast<int>(std::floor(inner.y - ow));
            int sw = static_cast<int>(std::ceil(rightEdge - (inner.x - ow)));
            int sh = static_cast<int>(std::ceil(inner.height + 2.0f * ow));
            BeginScissorMode(sx, sy, sw, sh);
            ui.Theme().Crystal(inner, style);
            if (highlighted && has)
            {
                ui.Theme().Chip(inner, Color{255, 255, 255, static_cast<unsigned char>(pulse * 150.0f)});
            }
            EndScissorMode();
        }
    }
}
}
