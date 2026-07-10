#ifndef VIEW_PREFAB_UI_UI_THEME_H
#define VIEW_PREFAB_UI_UI_THEME_H

#include "raylib.h"

#include "data/render/hud_params.h"

namespace view::ui
{
class UiTheme
{
public:
    void Load(Shader crystal, Texture2D cards, Texture2D white);
    void Unload();

    void Panel(Rectangle rect, Color tint) const;
    void Chip(Rectangle rect, Color tint) const;
    void Crystal(Rectangle rect, const data::CrystalStyle& style) const;
    void Fill(Rectangle rect, Color tint) const;

    const Texture2D& Cards() const { return cards_; }

private:
    Texture2D panel_{};
    Texture2D chip_{};
    Texture2D white_{};
    Texture2D cards_{};
    Shader crystalShader_{};
    int locTop_ = 0;
    int locBottom_ = 0;
    int locGloss_ = 0;
    int locSplit_ = 0;
    int locEdge_ = 0;
    NPatchInfo npatch_{};
    NPatchInfo chipNpatch_{};
    bool loaded_ = false;
};
}

#endif
