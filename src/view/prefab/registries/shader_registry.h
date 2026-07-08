#ifndef VIEW_PREFAB_REGISTRIES_SHADER_REGISTRY_H
#define VIEW_PREFAB_REGISTRIES_SHADER_REGISTRY_H

#include "raylib.h"

namespace view
{
class ShaderRegistry
{
public:
    void Load();
    void Unload();

    const Shader& Toon() const { return toon_; }
    const Shader& Geom() const { return geom_; }
    const Shader& Shadow() const { return shadow_; }
    const Shader& Outline() const { return outline_; }
    const Shader& Water() const { return water_; }
    const Shader& WaterLine() const { return waterLine_; }

private:
    Shader toon_{};
    Shader geom_{};
    Shader shadow_{};
    Shader outline_{};
    Shader water_{};
    Shader waterLine_{};
    bool loaded_ = false;
};
}

#endif
