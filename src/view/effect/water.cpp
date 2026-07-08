#include "view/effect/water.h"

#include "rlgl.h"

#include "view/effect/coast_sdf.h"
#include "view/prefab/model_registry.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    constexpr float WaterHeight = -0.1f;
    constexpr int SdfSlot = 11;
}

void WaterEffect::Load(const char* shaderDir, ModelRegistry& models)
{
    shader_ = LoadShader(TextFormat("%swater.vert", shaderDir), TextFormat("%swater.frag", shaderDir));
    sdfTexture_ = BuildCoastSdf(sdfOrigin_, sdfWorldSize_);

    float sdfMaxDist = SdfMaxDist;
    SetShaderValue(shader_, GetShaderLocation(shader_, "sdfOrigin"), &sdfOrigin_, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader_, GetShaderLocation(shader_, "sdfWorldSize"), &sdfWorldSize_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, GetShaderLocation(shader_, "sdfMaxDist"), &sdfMaxDist, SHADER_UNIFORM_FLOAT);

    timeLoc_ = GetShaderLocation(shader_, "time");
    sdfMapLoc_ = GetShaderLocation(shader_, "sdfMap");
    deepLoc_ = GetShaderLocation(shader_, "deepColor");
    shallowLoc_ = GetShaderLocation(shader_, "shallowColor");
    foamLoc_ = GetShaderLocation(shader_, "foamColor");
    outlineLoc_ = GetShaderLocation(shader_, "outlineColor");
    colorRangeLoc_ = GetShaderLocation(shader_, "colorRange");
    foamDistanceLoc_ = GetShaderLocation(shader_, "foamDistance");
    foamCutoffLoc_ = GetShaderLocation(shader_, "foamCutoff");
    noiseScaleLoc_ = GetShaderLocation(shader_, "noiseScale");
    distortAmountLoc_ = GetShaderLocation(shader_, "distortAmount");
    scrollSpeedLoc_ = GetShaderLocation(shader_, "scrollSpeed");
    outlineWidthLoc_ = GetShaderLocation(shader_, "outlineWidth");
    flowSpeedLoc_ = GetShaderLocation(shader_, "flowSpeed");
    flowAmountLoc_ = GetShaderLocation(shader_, "flowAmount");

    models.SetWaterShader(shader_);
}

void WaterEffect::Unload()
{
    UnloadTexture(sdfTexture_);
    UnloadShader(shader_);
}

void WaterEffect::Update(float time)
{
    SetShaderValue(shader_, timeLoc_, &time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, deepLoc_, &params_.deep, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader_, shallowLoc_, &params_.shallow, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader_, foamLoc_, &params_.foam, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader_, outlineLoc_, &params_.outline, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader_, colorRangeLoc_, &params_.colorRange, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, foamDistanceLoc_, &params_.foamDistance, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, foamCutoffLoc_, &params_.foamCutoff, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, noiseScaleLoc_, &params_.noiseScale, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, distortAmountLoc_, &params_.distortAmount, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, scrollSpeedLoc_, &params_.scrollSpeed, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, outlineWidthLoc_, &params_.outlineWidth, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, flowSpeedLoc_, &params_.flowSpeed, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, flowAmountLoc_, &params_.flowAmount, SHADER_UNIFORM_FLOAT);
}

void WaterEffect::Draw(const ModelRegistry& models) const
{
    int sdfSlot = SdfSlot;
    rlEnableShader(shader_.id);
    rlActiveTextureSlot(sdfSlot);
    rlEnableTexture(sdfTexture_.id);
    rlSetUniform(sdfMapLoc_, &sdfSlot, SHADER_UNIFORM_INT, 1);

    Vector3 center = SceneCenterWorld();
    DrawModel(models.Water(), Vector3{center.x, WaterHeight, center.z}, 1.0f, WHITE);
}
}
