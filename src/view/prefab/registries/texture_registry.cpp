#include "view/prefab/registries/texture_registry.h"

#include "data/card/card.h"

namespace view
{
void TextureRegistry::Load()
{
    cards_ = LoadTexture("assets/cards.png");
    SetTextureFilter(cards_, TEXTURE_FILTER_BILINEAR);

    Image w = GenImageColor(1, 1, WHITE);
    white_ = LoadTextureFromImage(w);
    UnloadImage(w);

    for (int i = 0; i < data::UnitTypeCount; i++)
    {
        data::CardDef def = data::CardDefOf(static_cast<data::UnitType>(i));
        previews_[static_cast<std::size_t>(i)] = LoadTexture(def.portrait);
        SetTextureFilter(previews_[static_cast<std::size_t>(i)], TEXTURE_FILTER_BILINEAR);
    }

    loaded_ = true;
}

void TextureRegistry::Unload()
{
    if (!loaded_) return;
    UnloadTexture(cards_);
    UnloadTexture(white_);
    for (Texture2D& tex : previews_) UnloadTexture(tex);
    loaded_ = false;
}

const Texture2D& TextureRegistry::Preview(data::UnitType type) const
{
    return previews_[static_cast<std::size_t>(type)];
}
}
