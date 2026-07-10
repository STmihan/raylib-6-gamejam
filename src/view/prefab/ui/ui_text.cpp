#include "view/prefab/ui/ui_text.h"

#include <sstream>

namespace view::ui
{
namespace
{
    constexpr int BaseSize = 64;
    constexpr float LineGap = 1.28f;
}

void TextRenderer::Load(Shader sdf)
{
    sdf_ = sdf;
    int fileSize = 0;
    unsigned char* fileData = LoadFileData("assets/fonts/Rubik-VariableFont_wght.ttf", &fileSize);

    int glyphCount = 0;
    font_.baseSize = BaseSize;
    font_.glyphs = LoadFontData(fileData, fileSize, BaseSize, nullptr, 0, FONT_SDF, &glyphCount);
    font_.glyphCount = glyphCount;
    Image atlas = GenImageFontAtlas(font_.glyphs, &font_.recs, glyphCount, BaseSize, 4, 1);
    font_.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    UnloadFileData(fileData);
    SetTextureFilter(font_.texture, TEXTURE_FILTER_BILINEAR);
    loaded_ = true;
}

void TextRenderer::Unload()
{
    if (!loaded_) return;
    UnloadFont(font_);
    loaded_ = false;
}

Vector2 TextRenderer::Measure(const char* text, float size, float spacing) const
{
    return MeasureTextEx(font_, text, size, spacing);
}

float TextRenderer::Width(const char* text, float size, float spacing) const
{
    return MeasureTextEx(font_, text, size, spacing).x;
}

float TextRenderer::LineHeight(float size) const
{
    return size * LineGap;
}

void TextRenderer::Draw(const char* text, Vector2 pos, float size, Color color, float spacing, bool bold) const
{
    BeginShaderMode(sdf_);
    if (bold)
    {
        float o = size * 0.035f;
        DrawTextEx(font_, text, Vector2{pos.x - o, pos.y}, size, spacing, color);
        DrawTextEx(font_, text, Vector2{pos.x + o, pos.y}, size, spacing, color);
        DrawTextEx(font_, text, Vector2{pos.x, pos.y - o}, size, spacing, color);
        DrawTextEx(font_, text, Vector2{pos.x, pos.y + o}, size, spacing, color);
    }
    DrawTextEx(font_, text, pos, size, spacing, color);
    EndShaderMode();
}

std::vector<std::string> TextRenderer::WrapLines(const std::string& text, float size, float maxWidth,
                                                 float spacing) const
{
    std::vector<std::string> lines;
    std::istringstream paragraphs(text);
    std::string paragraph;

    while (std::getline(paragraphs, paragraph, '\n'))
    {
        std::istringstream words(paragraph);
        std::string word;
        std::string line;
        while (words >> word)
        {
            std::string candidate = line.empty() ? word : line + " " + word;
            if (!line.empty() && Width(candidate.c_str(), size, spacing) > maxWidth)
            {
                lines.push_back(line);
                line = word;
            }
            else
            {
                line = candidate;
            }
        }
        lines.push_back(line);
    }
    return lines;
}

float TextRenderer::DrawWrapped(const std::string& text, Vector2 pos, float size, Color color, float maxWidth,
                                float spacing) const
{
    std::vector<std::string> lines = WrapLines(text, size, maxWidth, spacing);
    float lineHeight = LineHeight(size);
    float y = pos.y;
    for (const std::string& line : lines)
    {
        Draw(line.c_str(), Vector2{pos.x, y}, size, color, spacing);
        y += lineHeight;
    }
    return static_cast<float>(lines.size()) * lineHeight;
}
}
