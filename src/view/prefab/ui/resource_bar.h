#ifndef VIEW_PREFAB_UI_RESOURCE_BAR_H
#define VIEW_PREFAB_UI_RESOURCE_BAR_H

#include "raylib.h"

#include "data/render/hud_params.h"
#include "view/prefab/ui/ui_context.h"

namespace view::ui
{
void DrawResourceBar(UiContext& ui, Rectangle rect, float resource, int cap, int highlight, float time,
                     const data::CrystalStyle& style);
void DrawResourceBarSprite(UiContext& ui, Rectangle panel, float resource, int cap, int highlight, float time);
}

#endif
