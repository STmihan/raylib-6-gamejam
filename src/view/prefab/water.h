#ifndef VIEW_PREFAB_WATER_H
#define VIEW_PREFAB_WATER_H

#include "raylib.h"

#include "data/render/water_params.h"

namespace logic { struct Map; }

namespace view
{
class ModelRegistry;

class WaterEffect
{
public:
    void Load(Shader classic, Shader lines, ModelRegistry& models, const logic::Map& map);
    void Unload();

    void Update(float time);
    void Draw(const ModelRegistry& models) const;

    data::WaterParams& ParamsRef() { return params_; }

    int Mode() const { return mode_; }
    void SetMode(int mode);

    Texture2D SdfTexture() const { return sdfTexture_; }
    Vector2 SdfOrigin() const { return sdfOrigin_; }
    float SdfWorldSize() const { return sdfWorldSize_; }

private:
    void PushStatic(Shader shader) const;
    void CacheLocations(Shader shader);

    Shader classic_{};
    Shader lines_{};
    Shader active_{};
    ModelRegistry* models_ = nullptr;
    int mode_ = 1;

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

    int lineThickLoc_ = 0;
    int lineGapLoc_ = 0;
    int lineThinLoc_ = 0;
    int lineTravelLoc_ = 0;
    int lineSpeedLoc_ = 0;
    int lineIntervalLoc_ = 0;
    int lineWobbleLoc_ = 0;
    int lineWobbleScaleLoc_ = 0;
    int lineWobbleSpeedLoc_ = 0;
    int detailAmountLoc_ = 0;
    int detailScaleLoc_ = 0;
    int detailSpeedLoc_ = 0;
    int detailReachLoc_ = 0;
};
}

#endif
