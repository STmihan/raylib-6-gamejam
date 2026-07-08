#include "view/effect/outline_effect.h"

#include "raymath.h"
#include "rlgl.h"

#include "data/render/render_params.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/scene.h"

namespace view
{
void OutlineEffect::Init(Shader geomShader, Shader outlineShader)
{
    geomShader_ = geomShader;
    outlineShader_ = outlineShader;

    float maxDepth = data::Render.geomMaxDepth;
    SetShaderValue(geomShader_, GetShaderLocation(geomShader_, "maxDepth"), &maxDepth, SHADER_UNIFORM_FLOAT);

    int width = GetScreenWidth();
    int height = GetScreenHeight();
    Vector2 texelSize = {1.0f / static_cast<float>(width), 1.0f / static_cast<float>(height)};
    float outlineWidth = data::Render.outlineWidth;
    float creaseCos = data::Render.outlineCreaseCos;
    float depthThreshold = data::Render.outlineDepthThreshold;
    Vector4 outlineColor = data::Render.outlineColor;
    SetShaderValue(outlineShader_, GetShaderLocation(outlineShader_, "texelSize"), &texelSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(outlineShader_, GetShaderLocation(outlineShader_, "outlineWidth"), &outlineWidth,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, GetShaderLocation(outlineShader_, "creaseCos"), &creaseCos, SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, GetShaderLocation(outlineShader_, "depthThreshold"), &depthThreshold,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, GetShaderLocation(outlineShader_, "outlineColor"), &outlineColor,
                   SHADER_UNIFORM_VEC4);
    normalDepthLoc_ = GetShaderLocation(outlineShader_, "normalDepthTex");
    screenRightLoc_ = GetShaderLocation(outlineShader_, "screenRight");
    screenUpLoc_ = GetShaderLocation(outlineShader_, "screenUp");
    cavityRadiusLoc_ = GetShaderLocation(outlineShader_, "cavityRadius");
    cavityValleyLoc_ = GetShaderLocation(outlineShader_, "cavityValley");
    cavityRidgeLoc_ = GetShaderLocation(outlineShader_, "cavityRidge");

    normalDepthTarget_ = LoadRenderTexture(width, height);
}

void OutlineEffect::Shutdown()
{
    UnloadRenderTexture(normalDepthTarget_);
}

void OutlineEffect::RenderNormalDepth(ModelRegistry& models, const Scene& scene, const logic::Map& map,
                                      Camera3D camera)
{
    models.ApplyShader(geomShader_);
    BeginTextureMode(normalDepthTarget_);
    ClearBackground(BLANK);
    rlDisableColorBlend();
    BeginMode3D(camera);
    scene.Draw(map, false);
    EndMode3D();
    rlEnableColorBlend();
    EndTextureMode();
}

void OutlineEffect::Composite(RenderTexture2D colorTarget, const data::CavityParams& cavity, Camera3D camera)
{
    Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 camRight = Vector3Normalize(Vector3CrossProduct(camForward, camera.up));
    Vector3 camUp = Vector3Normalize(Vector3CrossProduct(camRight, camForward));
    float valley = cavity.enabled ? cavity.valley : 0.0f;
    float ridge = cavity.enabled ? cavity.ridge : 0.0f;
    SetShaderValue(outlineShader_, screenRightLoc_, &camRight, SHADER_UNIFORM_VEC3);
    SetShaderValue(outlineShader_, screenUpLoc_, &camUp, SHADER_UNIFORM_VEC3);
    SetShaderValue(outlineShader_, cavityRadiusLoc_, &cavity.radius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, cavityValleyLoc_, &valley, SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, cavityRidgeLoc_, &ridge, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(outlineShader_);
    SetShaderValueTexture(outlineShader_, normalDepthLoc_, normalDepthTarget_.texture);
    DrawTextureRec(
        colorTarget.texture,
        Rectangle{
            0.0f, 0.0f, static_cast<float>(colorTarget.texture.width),
            -static_cast<float>(colorTarget.texture.height)
        },
        Vector2{0.0f, 0.0f},
        WHITE);
    EndShaderMode();
}
}
