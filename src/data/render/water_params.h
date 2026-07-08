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
    float colorRange = 16.000f;
    float foamDistance = 0.512f;
    float foamCutoff = 0.609f;
    float noiseScale = 4.290f;
    float distortAmount = 0.500f;
    float scrollSpeed = 0.300f;
    float outlineWidth = 0.160f;
    float flowSpeed = 3.093f;
    float flowAmount = 0.267f;

    float lineThick = 0.214f;
    float lineGap = 0.151f;
    float lineThin = 0.046f;
    float lineTravel = 0.691f;
    float lineSpeed = 0.329f;
    float lineInterval = 2.486f;
    float lineWobble = 0.120f;
    float lineWobbleScale = 2.035f;
    float lineWobbleSpeed = 0.720f;
    float detailAmount = 0.058f;
    float detailScale = 10.667f;
    float detailSpeed = 0.018f;
    float detailReach = 2.176f;
};
}

#endif
