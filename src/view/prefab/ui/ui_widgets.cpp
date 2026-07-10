#include "view/prefab/ui/ui_widgets.h"

namespace view::ui
{
namespace
{
    Color Scale(Color c, float f)
    {
        auto ch = [f](unsigned char v) {
            float r = static_cast<float>(v) * f;
            return static_cast<unsigned char>(r > 255.0f ? 255.0f : r);
        };
        return Color{ch(c.r), ch(c.g), ch(c.b), c.a};
    }

    float Clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
}

void Panel(UiContext& ui, Rectangle rect, Color tint, bool blocking)
{
    ui.Theme().Panel(rect, tint);
    if (blocking) ui.Input().Block(rect);
}

bool ButtonBg(UiContext& ui, Rectangle rect, Color base)
{
    bool hover = ui.Input().Hover(rect);
    bool held = hover && ui.Input().Down();
    Color tint = held ? Scale(base, 0.8f) : (hover ? Scale(base, 1.18f) : base);
    ui.Theme().Panel(rect, tint);
    ui.Input().Block(rect);
    return ui.Input().Pressed(rect);
}

bool Button(UiContext& ui, Rectangle rect, const char* label, float fontSize)
{
    bool pressed = ButtonBg(ui, rect);
    LabelCentered(ui, label, rect, fontSize, Color{240, 236, 221, 255});
    return pressed;
}

void Bar(UiContext& ui, Rectangle rect, float frac, Color bg, Color fill)
{
    ui.Theme().Panel(rect, bg);
    const float pad = 3.0f;
    Rectangle inner = {rect.x + pad, rect.y + pad, (rect.width - 2.0f * pad) * Clamp01(frac),
                       rect.height - 2.0f * pad};
    ui.Theme().Fill(inner, fill);
}

void Icon(UiContext& ui, const Texture2D& tex, Rectangle rect, Color tint)
{
    float s = rect.width < rect.height ? rect.width : rect.height;
    float tw = static_cast<float>(tex.width);
    float th = static_cast<float>(tex.height);
    Rectangle src = {0.0f, 0.0f, tw, th};
    Rectangle dst = {rect.x + (rect.width - s) * 0.5f, rect.y + (rect.height - s) * 0.5f, s, s};
    DrawTexturePro(tex, src, dst, Vector2{0.0f, 0.0f}, 0.0f, tint);
}

void Label(UiContext& ui, const char* text, Vector2 pos, float size, Color color, bool bold)
{
    ui.Text().Draw(text, pos, size, color, 0.0f, bold);
}

void LabelCentered(UiContext& ui, const char* text, Rectangle rect, float size, Color color, bool bold)
{
    Vector2 sz = ui.Text().Measure(text, size);
    Vector2 pos = {rect.x + (rect.width - sz.x) * 0.5f, rect.y + (rect.height - sz.y) * 0.5f};
    ui.Text().Draw(text, pos, size, color, 0.0f, bold);
}
}
