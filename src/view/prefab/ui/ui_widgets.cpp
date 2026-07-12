#include "view/prefab/ui/ui_widgets.h"

#include <vector>

namespace view::ui
{
namespace
{
    struct RichItem
    {
        bool icon;
        std::string text;
    };

    std::vector<RichItem> ParseRich(const std::string& s)
    {
        std::vector<RichItem> items;
        std::string word;
        auto flush = [&] {
            if (!word.empty())
            {
                items.push_back({false, word});
                word.clear();
            }
        };
        for (std::size_t i = 0; i < s.size();)
        {
            char c = s[i];
            if (c == '[')
            {
                std::size_t close = s.find(']', i);
                if (close != std::string::npos)
                {
                    flush();
                    items.push_back({true, s.substr(i + 1, close - i - 1)});
                    i = close + 1;
                    continue;
                }
            }
            if (c == ' ' || c == '\n' || c == '\t') flush();
            else word.push_back(c);
            i++;
        }
        flush();
        return items;
    }

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

float DrawRichText(UiContext& ui, const std::string& text, Rectangle rect, float size, Color color,
                   Color iconTint, bool bold, float iconScale)
{
    std::vector<RichItem> items = ParseRich(text);
    float spaceW = ui.Text().Width(" ", size);
    float lineH = ui.Text().LineHeight(size);
    float iconSize = size * iconScale;
    float x = rect.x;
    float y = rect.y;
    bool lineStart = true;

    for (const RichItem& item : items)
    {
        bool asIcon = item.icon && ui.Atlas().Has(item.text);
        std::string glyphs = item.icon ? ("[" + item.text + "]") : item.text;
        float w = asIcon ? iconSize : ui.Text().Width(glyphs.c_str(), size);

        if (!lineStart && x + w > rect.x + rect.width)
        {
            x = rect.x;
            y += lineH;
            lineStart = true;
        }

        if (asIcon)
        {
            float iy = y + (size - iconSize) * 0.5f;
            ui.Atlas().DrawSprite(item.text, Rectangle{x, iy, iconSize, iconSize}, iconTint);
        }
        else
        {
            ui.Text().Draw(glyphs.c_str(), Vector2{x, y}, size, color, 0.0f, bold);
        }

        x += w + spaceW;
        lineStart = false;
    }

    return (y + lineH) - rect.y;
}
}
