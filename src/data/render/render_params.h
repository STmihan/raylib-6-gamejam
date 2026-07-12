#ifndef DATA_RENDER_RENDER_PARAMS_H
#define DATA_RENDER_RENDER_PARAMS_H

#include "raylib.h"

namespace data
{
struct RenderParams
{
    int shadowMapSize = 2048;
    float geomMaxDepth = 60.0f;

    float outlineWidth = 1.0f;
    float outlineCreaseCos = 0.5f;
    float outlineDepthThreshold = 0.02f;
    Vector4 outlineColor = {0.05f, 0.05f, 0.06f, 1.0f};

    Color backgroundColor = {33, 107, 168, 255};

    float lightScenePad = 3.0f;
    float lightDistMul = 2.2f;
    float lightFovMul = 2.0f;
    float lightExtMul = 1.05f;
    float lightNearFarPad = 2.0f;

    float shadowPreviewSize = 260.0f;
    float shadowPreviewMargin = 10.0f;

    int sdfResolution = 1024;
    float sdfMaxDist = 16.0f;
    float hexCircumradius = 1.0f;

    float waterHeight = -0.1f;
    float waterPlaneSize = 800.0f;
};

inline constexpr RenderParams Render{};
}

#endif
