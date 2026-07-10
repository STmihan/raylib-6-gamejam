#ifndef DATA_RENDER_HUD_PARAMS_H
#define DATA_RENDER_HUD_PARAMS_H

#include "raylib.h"

namespace data
{
struct CrystalStyle
{
    Vector3 top = {0.361f, 0.722f, 1.0f};
    Vector3 bottom = {0.102f, 0.420f, 0.859f};
    float gloss = 0.5f;
    float split = 0.561f;
    float edge = 0.1f;
    Vector3 outline = {0.0f, 0.0f, 0.0f};
    float outlineWidth = 1.39f;
};
}

#endif
