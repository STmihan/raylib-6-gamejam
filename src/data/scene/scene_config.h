#ifndef DATA_SCENE_SCENE_CONFIG_H
#define DATA_SCENE_SCENE_CONFIG_H

#include "raylib.h"

namespace data
{
inline constexpr int TreesPerForest = 3;
inline constexpr Vector3 TreeOffsets[TreesPerForest] = {
    {0.42f, 0.0f, 0.24f},
    {-0.42f, 0.0f, 0.24f},
    {0.0f, 0.0f, -0.42f},
};
inline constexpr int TreeSpeciesCount = 3;
inline constexpr int TreeSpeciesHashCol = 7;
inline constexpr int TreeSpeciesHashRow = 13;
inline constexpr int TreeYawHash = 57;

inline constexpr float BaseFlipDegrees = 180.0f;
}

#endif
