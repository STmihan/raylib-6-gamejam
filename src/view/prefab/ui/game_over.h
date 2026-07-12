#ifndef VIEW_PREFAB_UI_GAME_OVER_H
#define VIEW_PREFAB_UI_GAME_OVER_H

#include "view/prefab/ui/ui_context.h"

namespace logic { struct GameState; }

namespace view::ui
{
void DrawGameOver(UiContext& ui, const logic::GameState& state);
}

#endif
