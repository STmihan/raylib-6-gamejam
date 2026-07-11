#ifndef VIEW_EFFECT_COAST_SDF_H
#define VIEW_EFFECT_COAST_SDF_H

#include "raylib.h"

namespace logic { struct Map; }

namespace view
{
Texture2D BuildCoastSdf(Vector2& originOut, float& worldSizeOut, const logic::Map& map);
}

#endif
