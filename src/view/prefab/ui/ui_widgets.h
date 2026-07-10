#ifndef VIEW_PREFAB_UI_UI_WIDGETS_H
#define VIEW_PREFAB_UI_UI_WIDGETS_H

#include "raylib.h"

#include "view/prefab/ui/ui_context.h"

namespace view::ui
{
void Panel(UiContext& ui, Rectangle rect, Color tint, bool blocking = true);
bool ButtonBg(UiContext& ui, Rectangle rect, Color base = Color{70, 78, 60, 245});
bool Button(UiContext& ui, Rectangle rect, const char* label, float fontSize = 18.0f);
void Bar(UiContext& ui, Rectangle rect, float frac, Color bg, Color fill);
void Icon(UiContext& ui, const Texture2D& tex, Rectangle rect, Color tint);
void Label(UiContext& ui, const char* text, Vector2 pos, float size, Color color, bool bold = false);
void LabelCentered(UiContext& ui, const char* text, Rectangle rect, float size, Color color, bool bold = false);
}

#endif
