#ifndef VIEW_PREFAB_UI_UI_ATLAS_H
#define VIEW_PREFAB_UI_UI_ATLAS_H

#include <string>
#include <unordered_map>

#include "raylib.h"

namespace view::ui
{
class UiAtlas
{
public:
    void Load(const char* pngPath, const char* jsonPath);
    void Unload();
    bool Ready() const { return loaded_; }

    bool Has(const std::string& name) const { return slices_.count(name) > 0; }
    Rectangle Source(const std::string& name) const;

    void DrawSprite(const std::string& name, Rectangle dst, Color tint = WHITE) const;
    void DrawNPatch(const std::string& name, Rectangle dst, Color tint = WHITE) const;

private:
    struct Slice
    {
        Rectangle source{};
        bool ninePatch = false;
        NPatchInfo patch{};
    };

    Texture2D texture_{};
    std::unordered_map<std::string, Slice> slices_;
    bool loaded_ = false;
};
}

#endif
