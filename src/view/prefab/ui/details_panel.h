#ifndef VIEW_PREFAB_UI_DETAILS_PANEL_H
#define VIEW_PREFAB_UI_DETAILS_PANEL_H

#include "raylib.h"

#include "data/unit/unit_types.h"
#include "view/prefab/ui/ui_context.h"

namespace view
{
class TextureRegistry;
}

namespace view::ui
{
void DrawDetailsPanel(UiContext& ui, const TextureRegistry& textures, Rectangle rect, data::UnitType type,
                      int donor);
}

#endif
