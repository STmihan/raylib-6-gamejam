#include "view/prefab/registries/shader_registry.h"

namespace view
{
namespace
{
    const char* ShaderDir()
    {
#if defined(__EMSCRIPTEN__)
        return "assets/shaders/glsl100/";
#else
        return "assets/shaders/glsl330/";
#endif
    }
}

void ShaderRegistry::Load()
{
    const char* dir = ShaderDir();
    toon_ = LoadShader(TextFormat("%stoon.vert", dir), TextFormat("%stoon.frag", dir));
    geom_ = LoadShader(TextFormat("%sgeom.vert", dir), TextFormat("%sgeom.frag", dir));
    shadow_ = LoadShader(TextFormat("%sshadow.vert", dir), TextFormat("%sshadow.frag", dir));
    outline_ = LoadShader(0, TextFormat("%soutline.frag", dir));
    mask_ = LoadShader(TextFormat("%smask.vert", dir), TextFormat("%smask.frag", dir));
    water_ = LoadShader(TextFormat("%swater.vert", dir), TextFormat("%swater.frag", dir));
    waterLine_ = LoadShader(TextFormat("%swater.vert", dir), TextFormat("%swater_line.frag", dir));
    sdf_ = LoadShader(nullptr, TextFormat("%ssdf.frag", dir));
    crystal_ = LoadShader(nullptr, TextFormat("%scrystal.frag", dir));
    ring_ = LoadShader(nullptr, TextFormat("%sring.frag", dir));
    blob_ = LoadShader(TextFormat("%sblob.vert", dir), TextFormat("%sblob.frag", dir));
    vignette_ = LoadShader(nullptr, TextFormat("%svignette.frag", dir));
    loaded_ = true;
}

void ShaderRegistry::Unload()
{
    if (!loaded_) return;
    UnloadShader(vignette_);
    UnloadShader(blob_);
    UnloadShader(ring_);
    UnloadShader(crystal_);
    UnloadShader(sdf_);
    UnloadShader(waterLine_);
    UnloadShader(water_);
    UnloadShader(mask_);
    UnloadShader(outline_);
    UnloadShader(shadow_);
    UnloadShader(geom_);
    UnloadShader(toon_);
    loaded_ = false;
}
}
