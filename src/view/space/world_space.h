#ifndef VIEW_SPACE_WORLD_SPACE_H
#define VIEW_SPACE_WORLD_SPACE_H

#include "raylib.h"

#include "data/space/hex.h"
#include "data/space/vec.h"
#include "data/space/world_config.h"

namespace view
{
inline Vector3 LogicToWorld(data::Vec2 logic, float height)
{
    return {logic.x * data::RenderScale, height, logic.y * data::RenderScale};
}

inline Vector3 CellWorld(int col, int row, float height)
{
    return LogicToWorld(data::CellToLogic(col, row), height);
}

inline Vector3 Midpoint(Vector3 a, Vector3 b)
{
    return {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f, (a.z + b.z) * 0.5f};
}

inline Vector3 SceneCenterWorld()
{
    data::Vec2 center = data::FieldCenterLogic();
    return {center.x * data::RenderScale, 0.0f, center.y * data::RenderScale};
}

inline void DrawModelYaw(const Model& model, Vector3 position, float yawDegrees, Color tint)
{
    DrawModelEx(model, position, Vector3{0.0f, 1.0f, 0.0f}, yawDegrees, Vector3{1.0f, 1.0f, 1.0f}, tint);
}
}

#endif
