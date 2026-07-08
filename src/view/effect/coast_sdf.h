#ifndef VIEW_EFFECT_COAST_SDF_H
#define VIEW_EFFECT_COAST_SDF_H

#include "raylib.h"

namespace view
{
inline constexpr float SdfMaxDist = 16.0f;

Texture2D BuildCoastSdf(Vector2& originOut, float& worldSizeOut);
}

#endif
