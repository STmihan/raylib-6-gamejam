#include "view/effect/outline_effect.h"

#include "raymath.h"
#include "rlgl.h"

#include "data/render/render_params.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/scene.h"

namespace view
{
void OutlineEffect::Init(Shader geomShader, Shader outlineShader, Shader maskShader)
{
    geomShader_ = geomShader;
    outlineShader_ = outlineShader;
    maskShader_ = maskShader;

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
    unitMaskLoc_ = GetShaderLocation(outlineShader_, "unitMaskTex");
    screenRightLoc_ = GetShaderLocation(outlineShader_, "screenRight");
    screenUpLoc_ = GetShaderLocation(outlineShader_, "screenUp");
    cavityRadiusLoc_ = GetShaderLocation(outlineShader_, "cavityRadius");
    cavityValleyLoc_ = GetShaderLocation(outlineShader_, "cavityValley");
    cavityRidgeLoc_ = GetShaderLocation(outlineShader_, "cavityRidge");
    unitOutlineScaleLoc_ = GetShaderLocation(outlineShader_, "unitOutlineScale");
    unitCavityScaleLoc_ = GetShaderLocation(outlineShader_, "unitCavityScale");

    normalDepthTarget_ = LoadRenderTexture(width, height);
    maskTarget_ = LoadRenderTexture(width, height);
}

void OutlineEffect::Shutdown()
{
    UnloadRenderTexture(normalDepthTarget_);
    UnloadRenderTexture(maskTarget_);
}

void OutlineEffect::RenderNormalDepth(ModelRegistry& models, Camera3D camera,
                                      const std::function<void()>& drawScene)
{
    models.ApplyShader(geomShader_);
    BeginTextureMode(normalDepthTarget_);
    ClearBackground(BLANK);
    rlDisableColorBlend();
    BeginMode3D(camera);
    if (drawScene) drawScene();
    EndMode3D();
    rlEnableColorBlend();
    EndTextureMode();
}

void OutlineEffect::RenderUnitMask(ModelRegistry& models, Camera3D camera, const std::function<void()>& drawExtra)
{
    models.ApplyShader(maskShader_);
    BeginTextureMode(maskTarget_);
    ClearBackground(BLACK);
    BeginMode3D(camera);
    if (drawExtra) drawExtra();
    EndMode3D();
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
    SetShaderValue(outlineShader_, unitOutlineScaleLoc_, &unitOutlineScale_, SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, unitCavityScaleLoc_, &unitCavityScale_, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(outlineShader_);
    SetShaderValueTexture(outlineShader_, normalDepthLoc_, normalDepthTarget_.texture);
    SetShaderValueTexture(outlineShader_, unitMaskLoc_, maskTarget_.texture);
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
