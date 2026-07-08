#include "view/prefab/water.h"

#include "rlgl.h"

#include "data/render/render_params.h"
#include "view/prefab/coast_sdf.h"
#include "view/prefab/registries/model_registry.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    constexpr int SdfSlot = 11;
}

void WaterEffect::Load(Shader classic, Shader lines, ModelRegistry& models)
{
    classic_ = classic;
    lines_ = lines;
    models_ = &models;
    sdfTexture_ = BuildCoastSdf(sdfOrigin_, sdfWorldSize_);

    PushStatic(classic_);
    PushStatic(lines_);

    active_ = mode_ == 1 ? lines_ : classic_;
    CacheLocations(active_);
    models.SetWaterShader(active_);
}

void WaterEffect::PushStatic(Shader shader) const
{
    float sdfMaxDist = data::Render.sdfMaxDist;
    SetShaderValue(shader, GetShaderLocation(shader, "sdfOrigin"), &sdfOrigin_, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, GetShaderLocation(shader, "sdfWorldSize"), &sdfWorldSize_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, GetShaderLocation(shader, "sdfMaxDist"), &sdfMaxDist, SHADER_UNIFORM_FLOAT);
}

void WaterEffect::CacheLocations(Shader shader)
{
    timeLoc_ = GetShaderLocation(shader, "time");
    sdfMapLoc_ = GetShaderLocation(shader, "sdfMap");
    deepLoc_ = GetShaderLocation(shader, "deepColor");
    shallowLoc_ = GetShaderLocation(shader, "shallowColor");
    foamLoc_ = GetShaderLocation(shader, "foamColor");
    outlineLoc_ = GetShaderLocation(shader, "outlineColor");
    colorRangeLoc_ = GetShaderLocation(shader, "colorRange");
    foamDistanceLoc_ = GetShaderLocation(shader, "foamDistance");
    foamCutoffLoc_ = GetShaderLocation(shader, "foamCutoff");
    noiseScaleLoc_ = GetShaderLocation(shader, "noiseScale");
    distortAmountLoc_ = GetShaderLocation(shader, "distortAmount");
    scrollSpeedLoc_ = GetShaderLocation(shader, "scrollSpeed");
    outlineWidthLoc_ = GetShaderLocation(shader, "outlineWidth");
    flowSpeedLoc_ = GetShaderLocation(shader, "flowSpeed");
    flowAmountLoc_ = GetShaderLocation(shader, "flowAmount");

    lineThickLoc_ = GetShaderLocation(shader, "lineThick");
    lineGapLoc_ = GetShaderLocation(shader, "lineGap");
    lineThinLoc_ = GetShaderLocation(shader, "lineThin");
    lineTravelLoc_ = GetShaderLocation(shader, "lineTravel");
    lineSpeedLoc_ = GetShaderLocation(shader, "lineSpeed");
    lineIntervalLoc_ = GetShaderLocation(shader, "lineInterval");
    lineWobbleLoc_ = GetShaderLocation(shader, "lineWobble");
    lineWobbleScaleLoc_ = GetShaderLocation(shader, "lineWobbleScale");
    lineWobbleSpeedLoc_ = GetShaderLocation(shader, "lineWobbleSpeed");
    detailAmountLoc_ = GetShaderLocation(shader, "detailAmount");
    detailScaleLoc_ = GetShaderLocation(shader, "detailScale");
    detailSpeedLoc_ = GetShaderLocation(shader, "detailSpeed");
    detailReachLoc_ = GetShaderLocation(shader, "detailReach");
}

void WaterEffect::SetMode(int mode)
{
    if (mode == mode_) return;
    mode_ = mode;
    active_ = mode_ == 1 ? lines_ : classic_;
    CacheLocations(active_);
    if (models_) models_->SetWaterShader(active_);
}

void WaterEffect::Unload()
{
    UnloadTexture(sdfTexture_);
}

void WaterEffect::Update(float time)
{
    SetShaderValue(active_, timeLoc_, &time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, deepLoc_, &params_.deep, SHADER_UNIFORM_VEC3);
    SetShaderValue(active_, shallowLoc_, &params_.shallow, SHADER_UNIFORM_VEC3);
    SetShaderValue(active_, foamLoc_, &params_.foam, SHADER_UNIFORM_VEC3);
    SetShaderValue(active_, outlineLoc_, &params_.outline, SHADER_UNIFORM_VEC3);
    SetShaderValue(active_, colorRangeLoc_, &params_.colorRange, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, foamDistanceLoc_, &params_.foamDistance, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, foamCutoffLoc_, &params_.foamCutoff, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, noiseScaleLoc_, &params_.noiseScale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, distortAmountLoc_, &params_.distortAmount, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, scrollSpeedLoc_, &params_.scrollSpeed, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, outlineWidthLoc_, &params_.outlineWidth, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, flowSpeedLoc_, &params_.flowSpeed, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, flowAmountLoc_, &params_.flowAmount, SHADER_UNIFORM_FLOAT);

    SetShaderValue(active_, lineThickLoc_, &params_.lineThick, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, lineGapLoc_, &params_.lineGap, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, lineThinLoc_, &params_.lineThin, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, lineTravelLoc_, &params_.lineTravel, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, lineSpeedLoc_, &params_.lineSpeed, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, lineIntervalLoc_, &params_.lineInterval, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, lineWobbleLoc_, &params_.lineWobble, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, lineWobbleScaleLoc_, &params_.lineWobbleScale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, lineWobbleSpeedLoc_, &params_.lineWobbleSpeed, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, detailAmountLoc_, &params_.detailAmount, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, detailScaleLoc_, &params_.detailScale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, detailSpeedLoc_, &params_.detailSpeed, SHADER_UNIFORM_FLOAT);
    SetShaderValue(active_, detailReachLoc_, &params_.detailReach, SHADER_UNIFORM_FLOAT);
}

void WaterEffect::Draw(const ModelRegistry& models) const
{
    int sdfSlot = SdfSlot;
    rlEnableShader(active_.id);
    rlActiveTextureSlot(sdfSlot);
    rlEnableTexture(sdfTexture_.id);
    rlSetUniform(sdfMapLoc_, &sdfSlot, SHADER_UNIFORM_INT, 1);

    Vector3 center = SceneCenterWorld();
    DrawModel(models.Water(), Vector3{center.x, data::Render.waterHeight, center.z}, 1.0f, WHITE);
}
}
