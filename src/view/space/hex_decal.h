#ifndef VIEW_SPACE_HEX_DECAL_H
#define VIEW_SPACE_HEX_DECAL_H

#include <cmath>

#include "raylib.h"

#include "data/space/hex.h"

namespace view
{
inline constexpr int HexEdgeCorners[6][2] = {{5, 0}, {4, 5}, {3, 4}, {2, 3}, {1, 2}, {0, 1}};

inline Vector3 HexCorner(Vector3 center, int k)
{
    static const float cornerX[6] = {0.8660254f, 0.0f, -0.8660254f, -0.8660254f, 0.0f, 0.8660254f};
    static const float cornerZ[6] = {0.5f, 1.0f, 0.5f, -0.5f, -1.0f, -0.5f};
    return Vector3{center.x + cornerX[k] * data::HexRadius, center.y, center.z + cornerZ[k] * data::HexRadius};
}

inline void DrawHexEdge(Vector3 a, Vector3 b, float width, Color color)
{
    float dx = b.x - a.x;
    float dz = b.z - a.z;
    float len = std::sqrt(dx * dx + dz * dz);
    if (len < 1e-4f) return;
    float px = (dz / len) * (width * 0.5f);
    float pz = (-dx / len) * (width * 0.5f);
    Vector3 a1 = {a.x + px, a.y, a.z + pz};
    Vector3 a2 = {a.x - px, a.y, a.z - pz};
    Vector3 b1 = {b.x + px, b.y, b.z + pz};
    Vector3 b2 = {b.x - px, b.y, b.z - pz};
    DrawTriangle3D(a1, b1, b2, color);
    DrawTriangle3D(a1, b2, a2, color);
    DrawTriangle3D(b2, b1, a1, color);
    DrawTriangle3D(a2, b2, a1, color);
}

inline void DrawHexOutline(Vector3 center, float width, Color color)
{
    for (int dir = 0; dir < 6; dir++)
    {
        DrawHexEdge(HexCorner(center, HexEdgeCorners[dir][0]), HexCorner(center, HexEdgeCorners[dir][1]), width,
                    color);
    }
}
}

#endif
