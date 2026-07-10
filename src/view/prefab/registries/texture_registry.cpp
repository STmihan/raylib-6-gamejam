#include "view/prefab/registries/texture_registry.h"

namespace view
{
void TextureRegistry::Load()
{
    cards_ = LoadTexture("assets/cards.png");
    SetTextureFilter(cards_, TEXTURE_FILTER_BILINEAR);

    Image w = GenImageColor(1, 1, WHITE);
    white_ = LoadTextureFromImage(w);
    UnloadImage(w);

    loaded_ = true;
}

void TextureRegistry::Unload()
{
    if (!loaded_) return;
    UnloadTexture(cards_);
    UnloadTexture(white_);
    loaded_ = false;
}
}
