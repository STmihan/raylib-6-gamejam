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

struct HandParams
{
    Vector2 pivot = {160.0f, 1110.0f};
    float radius = 460.0f;
    float step = 11.0f;
    float raise = 20.0f;
    float hoverScale = 0.14f;
    float hoverTime = 0.10f;
    Vector2 anchor = {160.0f, 636.0f};
    float dragRadius = 68.0f;
};
}

#endif
