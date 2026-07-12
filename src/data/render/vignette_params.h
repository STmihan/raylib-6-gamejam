#ifndef DATA_RENDER_VIGNETTE_PARAMS_H
#define DATA_RENDER_VIGNETTE_PARAMS_H

#include "raylib.h"

namespace data
{
struct VignetteParams
{
    bool enabled = false;
    Vector3 color = {0.72f, 0.04f, 0.04f};
    float intensity = 0.4f;
    float radius = 0.5f;
    float softness = 0.5f;
};
}

#endif
