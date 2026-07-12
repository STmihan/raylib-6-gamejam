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
    const Shader& Mask() const { return mask_; }
    const Shader& Water() const { return water_; }
    const Shader& WaterLine() const { return waterLine_; }
    const Shader& Sdf() const { return sdf_; }
    const Shader& Crystal() const { return crystal_; }
    const Shader& Ring() const { return ring_; }
    const Shader& Blob() const { return blob_; }
    const Shader& Vignette() const { return vignette_; }

private:
    Shader toon_{};
    Shader geom_{};
    Shader shadow_{};
    Shader outline_{};
    Shader mask_{};
    Shader water_{};
    Shader waterLine_{};
    Shader sdf_{};
    Shader crystal_{};
    Shader ring_{};
    Shader blob_{};
    Shader vignette_{};
    bool loaded_ = false;
};
}

#endif
