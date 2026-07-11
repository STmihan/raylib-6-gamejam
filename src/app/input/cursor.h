#ifndef APP_INPUT_CURSOR_H
#define APP_INPUT_CURSOR_H

#include "raylib.h"

#include "data/space/hex.h"
#include "data/space/vec.h"

namespace app
{
inline bool CursorLogic(Camera3D camera, Vector2 screen, data::Vec2& out)
{
    Ray ray = GetScreenToWorldRay(screen, camera);
    if (ray.direction.y > -0.0001f) return false;
    float t = -ray.position.y / ray.direction.y;
    out = {(ray.position.x + ray.direction.x * t) / data::RenderScale,
           (ray.position.z + ray.direction.z * t) / data::RenderScale};
    return true;
}
}

#endif
