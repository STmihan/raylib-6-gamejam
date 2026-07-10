#ifndef VIEW_PREFAB_REGISTRIES_TEXTURE_REGISTRY_H
#define VIEW_PREFAB_REGISTRIES_TEXTURE_REGISTRY_H

#include "raylib.h"

namespace view
{
class TextureRegistry
{
public:
    void Load();
    void Unload();

    const Texture2D& Cards() const { return cards_; }
    const Texture2D& White() const { return white_; }

private:
    Texture2D cards_{};
    Texture2D white_{};
    bool loaded_ = false;
};
}

#endif
