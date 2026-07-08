#ifndef VIEW_RENDER_SHADOW_PARAMS_H
#define VIEW_RENDER_SHADOW_PARAMS_H

#include "raylib.h"

namespace view
{
struct ShadowParams
{
    Vector3 sunDir = {-0.5f, 0.6f, -0.7f};
    float ambient = 1.0f;
    float bands = 3.0f;
    float shadowStrength = 0.4f;
    float softness = 1.564f;
    float biasSlope = 0.0f;
    float biasConstant = 0.005f;
};
}

#endif
