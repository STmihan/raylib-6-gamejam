#ifndef VIEW_PREFAB_REGISTRIES_TEXTURE_REGISTRY_H
#define VIEW_PREFAB_REGISTRIES_TEXTURE_REGISTRY_H

#include <array>

#include "raylib.h"

#include "data/unit/unit.h"

namespace view
{
class TextureRegistry
{
public:
    void Load();
    void Unload();

    const Texture2D& Cards() const { return cards_; }
    const Texture2D& White() const { return white_; }
    const Texture2D& Preview(data::UnitType type) const;
    const Texture2D& MergeIcon(data::UnitType type) const;

private:
    Texture2D cards_{};
    Texture2D white_{};
    std::array<Texture2D, data::UnitTypeCount> previews_{};
    std::array<Texture2D, data::UnitTypeCount> mergeIcons_{};
    bool loaded_ = false;
};
}

#endif
