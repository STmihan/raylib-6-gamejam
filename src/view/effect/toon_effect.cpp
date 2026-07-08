#include "view/effect/toon_effect.h"

#include "rlgl.h"

#include "data/render/render_params.h"
#include "view/prefab/registries/model_registry.h"

namespace view
{
void ToonEffect::Init(Shader shader)
{
    shader_ = shader;
    float shadowTexel = 1.0f / static_cast<float>(data::Render.shadowMapSize);
    SetShaderValue(shader_, GetShaderLocation(shader_, "shadowTexel"), &shadowTexel, SHADER_UNIFORM_FLOAT);
    sunDirLoc_ = GetShaderLocation(shader_, "sunDir");
    ambientLoc_ = GetShaderLocation(shader_, "ambient");
    bandsLoc_ = GetShaderLocation(shader_, "bands");
    shadowStrengthLoc_ = GetShaderLocation(shader_, "shadowStrength");
    softnessLoc_ = GetShaderLocation(shader_, "shadowSoftness");
    biasSlopeLoc_ = GetShaderLocation(shader_, "biasSlope");
    biasConstantLoc_ = GetShaderLocation(shader_, "biasConstant");
    shadowMapLoc_ = GetShaderLocation(shader_, "shadowMap");
    lightViewProjLoc_ = GetShaderLocation(shader_, "lightViewProj");
}

void ToonEffect::Apply(ModelRegistry& models) const
{
    models.ApplyShader(shader_);
}

void ToonEffect::Upload(const data::ShadowParams& params, Matrix lightViewProj)
{
    SetShaderValue(shader_, sunDirLoc_, &params.sunDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader_, ambientLoc_, &params.ambient, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, bandsLoc_, &params.bands, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, shadowStrengthLoc_, &params.shadowStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, softnessLoc_, &params.softness, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, biasSlopeLoc_, &params.biasSlope, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader_, biasConstantLoc_, &params.biasConstant, SHADER_UNIFORM_FLOAT);
    SetShaderValueMatrix(shader_, lightViewProjLoc_, lightViewProj);
}

void ToonEffect::BindShadowMap(unsigned int depthTextureId, int slot)
{
    rlEnableShader(shader_.id);
    rlActiveTextureSlot(slot);
    rlEnableTexture(depthTextureId);
    rlSetUniform(shadowMapLoc_, &slot, SHADER_UNIFORM_INT, 1);
}
}
