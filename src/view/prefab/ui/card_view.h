#ifndef VIEW_PREFAB_UI_CARD_VIEW_H
#define VIEW_PREFAB_UI_CARD_VIEW_H

#include "raylib.h"

#include "data/card/card.h"
#include "view/prefab/ui/ui_context.h"

namespace view::ui
{
void DrawCard(UiContext& ui, Rectangle rect, const data::CardDef& def, const Texture2D& portrait,
              int chargesLeft, int cost, int donor, const Texture2D& donorIcon);
void DrawCardTransformed(UiContext& ui, RenderTexture2D& target, const data::CardDef& def,
                         const Texture2D& portrait, Vector2 center, float rotationDeg, float scale,
                         float cardW, float cardH, int chargesLeft, int cost, int donor,
                         const Texture2D& donorIcon, bool useAtlas);
}

#endif
