#ifndef VIEW_PREFAB_UI_MATCH_TIMER_H
#define VIEW_PREFAB_UI_MATCH_TIMER_H

#include <cstdint>

#include "raylib.h"

namespace view::ui
{
class UiContext;

void DrawMatchTimer(UiContext& ui, Rectangle rect, std::uint64_t tick, bool useAtlas);
}

#endif
