#ifndef VIEW_PREFAB_UI_UI_TEXT_H
#define VIEW_PREFAB_UI_UI_TEXT_H

#include <string>
#include <vector>

#include "raylib.h"

namespace view::ui
{
class TextRenderer
{
public:
    void Load(Shader sdf);
    void Unload();

    Vector2 Measure(const char* text, float size, float spacing = 0.0f) const;
    float Width(const char* text, float size, float spacing = 0.0f) const;
    float LineHeight(float size) const;

    void Draw(const char* text, Vector2 pos, float size, Color color, float spacing = 0.0f,
              bool bold = false) const;
    void DrawRotated(const char* text, Vector2 pos, Vector2 origin, float rotationDeg, float size,
                     Color color, float spacing = 0.0f, bool bold = false) const;

    std::vector<std::string> WrapLines(const std::string& text, float size, float maxWidth,
                                       float spacing = 0.0f) const;
    float DrawWrapped(const std::string& text, Vector2 pos, float size, Color color, float maxWidth,
                      float spacing = 0.0f) const;

    const Font& FontRef() const { return font_; }

private:
    Font font_{};
    Shader sdf_{};
    bool loaded_ = false;
};
}

#endif
