#ifndef VIEW_SPACE_ORIENTATION_H
#define VIEW_SPACE_ORIENTATION_H

#include <cmath>

#include "raylib.h"
#include "raymath.h"

#include "data/unit/unit.h"

namespace view
{
inline float TeamYaw(data::Team team)
{
    return team == data::Team::Top ? 180.0f : 0.0f;
}

inline float YawTowards(Vector2 from, Vector2 to, float fallback)
{
    float dx = to.x - from.x;
    float dz = to.y - from.y;
    if (dx * dx + dz * dz < 1e-6f) return fallback;
    return std::atan2(-dx, -dz) * RAD2DEG;
}
}

#endif
