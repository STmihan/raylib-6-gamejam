#include "view/prefab/ui/ui_theme.h"

namespace view::ui
{
namespace
{
    constexpr int Size = 40;
    constexpr int Radius = 8;
    constexpr int Border = 3;
    constexpr int Margin = 12;

    void RoundRect(Image* img, int x, int y, int w, int h, int r, Color c)
    {
        ImageDrawRectangleRec(img, Rectangle{static_cast<float>(x + r), static_cast<float>(y),
                                             static_cast<float>(w - 2 * r), static_cast<float>(h)}, c);
        ImageDrawRectangleRec(img, Rectangle{static_cast<float>(x), static_cast<float>(y + r),
                                             static_cast<float>(w), static_cast<float>(h - 2 * r)}, c);
        ImageDrawCircle(img, x + r, y + r, r, c);
        ImageDrawCircle(img, x + w - r - 1, y + r, r, c);
        ImageDrawCircle(img, x + r, y + h - r - 1, r, c);
        ImageDrawCircle(img, x + w - r - 1, y + h - r - 1, r, c);
    }
}

void UiTheme::Load(Shader crystal, Texture2D cards, Texture2D white)
{
    crystalShader_ = crystal;
    cards_ = cards;
    white_ = white;

    Image img = GenImageColor(Size, Size, BLANK);
    RoundRect(&img, 0, 0, Size, Size, Radius, Color{120, 120, 120, 255});
    RoundRect(&img, Border, Border, Size - 2 * Border, Size - 2 * Border, Radius - Border,
              Color{232, 232, 232, 255});
    panel_ = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(panel_, TEXTURE_FILTER_BILINEAR);

    const int ChipSize = 32;
    const int ChipRadius = 6;
    const int ChipMargin = 8;

    Image chip = GenImageColor(ChipSize, ChipSize, BLANK);
    RoundRect(&chip, 0, 0, ChipSize, ChipSize, ChipRadius, WHITE);
    chip_ = LoadTextureFromImage(chip);
    UnloadImage(chip);
    SetTextureFilter(chip_, TEXTURE_FILTER_BILINEAR);

    locTop_ = GetShaderLocation(crystalShader_, "colorTop");
    locBottom_ = GetShaderLocation(crystalShader_, "colorBottom");
    locGloss_ = GetShaderLocation(crystalShader_, "gloss");
    locSplit_ = GetShaderLocation(crystalShader_, "colorSplit");
    locEdge_ = GetShaderLocation(crystalShader_, "colorEdge");

    npatch_ = NPatchInfo{Rectangle{0.0f, 0.0f, static_cast<float>(Size), static_cast<float>(Size)},
                         Margin, Margin, Margin, Margin, NPATCH_NINE_PATCH};
    chipNpatch_ = NPatchInfo{Rectangle{0.0f, 0.0f, static_cast<float>(ChipSize), static_cast<float>(ChipSize)},
                             ChipMargin, ChipMargin, ChipMargin, ChipMargin, NPATCH_NINE_PATCH};
    loaded_ = true;
}

void UiTheme::Unload()
{
    if (!loaded_) return;
    UnloadTexture(panel_);
    UnloadTexture(chip_);
    loaded_ = false;
}

void UiTheme::Panel(Rectangle rect, Color tint) const
{
    DrawTextureNPatch(panel_, npatch_, rect, Vector2{0.0f, 0.0f}, 0.0f, tint);
}

void UiTheme::Chip(Rectangle rect, Color tint) const
{
    DrawTextureNPatch(chip_, chipNpatch_, rect, Vector2{0.0f, 0.0f}, 0.0f, tint);
}

void UiTheme::Crystal(Rectangle rect, const data::CrystalStyle& style) const
{
    if (style.outlineWidth > 0.0f)
    {
        Rectangle o = {rect.x - style.outlineWidth, rect.y - style.outlineWidth,
                       rect.width + 2.0f * style.outlineWidth, rect.height + 2.0f * style.outlineWidth};
        Color outline = {static_cast<unsigned char>(style.outline.x * 255.0f),
                         static_cast<unsigned char>(style.outline.y * 255.0f),
                         static_cast<unsigned char>(style.outline.z * 255.0f), 255};
        Chip(o, outline);
    }

    SetShaderValue(crystalShader_, locTop_, &style.top, SHADER_UNIFORM_VEC3);
    SetShaderValue(crystalShader_, locBottom_, &style.bottom, SHADER_UNIFORM_VEC3);
    SetShaderValue(crystalShader_, locGloss_, &style.gloss, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crystalShader_, locSplit_, &style.split, SHADER_UNIFORM_FLOAT);
    SetShaderValue(crystalShader_, locEdge_, &style.edge, SHADER_UNIFORM_FLOAT);
    BeginShaderMode(crystalShader_);
    Rectangle src = {0.0f, 0.0f, static_cast<float>(chip_.width), static_cast<float>(chip_.height)};
    DrawTexturePro(chip_, src, rect, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    EndShaderMode();
}

void UiTheme::Fill(Rectangle rect, Color tint) const
{
    DrawTexturePro(white_, Rectangle{0.0f, 0.0f, 1.0f, 1.0f}, rect, Vector2{0.0f, 0.0f}, 0.0f, tint);
}
}
