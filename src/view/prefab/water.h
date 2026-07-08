#ifndef VIEW_PREFAB_WATER_H
#define VIEW_PREFAB_WATER_H

#include "raylib.h"

#include "data/render/water_params.h"

namespace view
{
class ModelRegistry;

class WaterEffect
{
public:
    void Load(Shader shader, ModelRegistry& models);
    void Unload();

    void Update(float time);
    void Draw(const ModelRegistry& models) const;

    data::WaterParams& ParamsRef() { return params_; }

private:
    Shader shader_{};
    data::WaterParams params_{};
    Texture2D sdfTexture_{};
    Vector2 sdfOrigin_{};
    float sdfWorldSize_ = 1.0f;

    int timeLoc_ = 0;
    int sdfMapLoc_ = 0;
    int deepLoc_ = 0;
    int shallowLoc_ = 0;
    int foamLoc_ = 0;
    int outlineLoc_ = 0;
    int colorRangeLoc_ = 0;
    int foamDistanceLoc_ = 0;
    int foamCutoffLoc_ = 0;
    int noiseScaleLoc_ = 0;
    int distortAmountLoc_ = 0;
    int scrollSpeedLoc_ = 0;
    int outlineWidthLoc_ = 0;
    int flowSpeedLoc_ = 0;
    int flowAmountLoc_ = 0;
};
}

#endif
