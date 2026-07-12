#include "view/prefab/ui/ui_atlas.h"

#include <fstream>

#include <nlohmann/json.hpp>

namespace view::ui
{
void UiAtlas::Load(const char* pngPath, const char* jsonPath)
{
    texture_ = LoadTexture(pngPath);
    SetTextureFilter(texture_, TEXTURE_FILTER_POINT);

    std::ifstream file(jsonPath);
    if (!file) return;

    nlohmann::json root;
    try
    {
        file >> root;
    }
    catch (const nlohmann::json::exception&)
    {
        return;
    }

    if (!root.contains("meta") || !root["meta"].contains("slices")) return;
    for (const nlohmann::json& s : root["meta"]["slices"])
    {
        if (!s.contains("name") || !s.contains("keys") || s["keys"].empty()) continue;
        const nlohmann::json& key = s["keys"][0];
        if (!key.contains("bounds")) continue;
        const nlohmann::json& b = key["bounds"];

        Slice slice;
        slice.source = {static_cast<float>(b["x"]), static_cast<float>(b["y"]), static_cast<float>(b["w"]),
                        static_cast<float>(b["h"])};
        if (key.contains("center"))
        {
            const nlohmann::json& c = key["center"];
            int cx = c["x"];
            int cy = c["y"];
            int cw = c["w"];
            int ch = c["h"];
            int bw = b["w"];
            int bh = b["h"];
            slice.ninePatch = true;
            slice.patch.source = slice.source;
            slice.patch.left = cx;
            slice.patch.top = cy;
            slice.patch.right = bw - (cx + cw);
            slice.patch.bottom = bh - (cy + ch);
            slice.patch.layout = NPATCH_NINE_PATCH;
        }
        slices_[s["name"].get<std::string>()] = slice;
    }
    loaded_ = true;
}

void UiAtlas::Unload()
{
    if (!loaded_) return;
    UnloadTexture(texture_);
    slices_.clear();
    loaded_ = false;
}

Rectangle UiAtlas::Source(const std::string& name) const
{
    auto it = slices_.find(name);
    return it != slices_.end() ? it->second.source : Rectangle{0.0f, 0.0f, 0.0f, 0.0f};
}

void UiAtlas::DrawSprite(const std::string& name, Rectangle dst, Color tint) const
{
    auto it = slices_.find(name);
    if (it == slices_.end()) return;
    DrawTexturePro(texture_, it->second.source, dst, Vector2{0.0f, 0.0f}, 0.0f, tint);
}

void UiAtlas::DrawNPatch(const std::string& name, Rectangle dst, Color tint) const
{
    auto it = slices_.find(name);
    if (it == slices_.end()) return;
    if (it->second.ninePatch)
    {
        DrawTextureNPatch(texture_, it->second.patch, dst, Vector2{0.0f, 0.0f}, 0.0f, tint);
    }
    else
    {
        DrawTexturePro(texture_, it->second.source, dst, Vector2{0.0f, 0.0f}, 0.0f, tint);
    }
}
}
