#ifndef DATA_RENDER_WATER_PARAMS_H
#define DATA_RENDER_WATER_PARAMS_H

#include "raylib.h"

namespace data
{
struct WaterParams
{
    Vector3 deep = {0.130f, 0.420f, 0.660f};
    Vector3 shallow = {0.480f, 0.800f, 0.870f};
    Vector3 foam = {0.950f, 0.980f, 1.000f};
    Vector3 outline = {0.030f, 0.030f, 0.050f};
    float colorRange = 8.147f;
    float foamDistance = 0.512f;
    float foamCutoff = 0.609f;
    float noiseScale = 4.290f;
    float distortAmount = 0.500f;
    float scrollSpeed = 0.300f;
    float outlineWidth = 0.160f;
    float flowSpeed = 3.093f;
    float flowAmount = 0.267f;
};
}

#endif
