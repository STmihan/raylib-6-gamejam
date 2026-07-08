#include "view/render/renderer.h"

#include <cmath>

#include "raymath.h"
#include "rlgl.h"

#include "logic/world/map.h"
#include "view/prefab/scene.h"
#include "view/render/render_targets.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    constexpr int ShadowMapSize = 2048;
    constexpr int ShadowSlot = 10;

    const char* ShaderDir()
    {
#if defined(__EMSCRIPTEN__)
        return "assets/shaders/glsl100/";
#else
        return "assets/shaders/glsl330/";
#endif
    }
}

void Renderer::Init()
{
    models_.Load();

    const char* dir = ShaderDir();
    toonShader_ = LoadShader(TextFormat("%stoon.vert", dir), TextFormat("%stoon.frag", dir));
    geomShader_ = LoadShader(TextFormat("%sgeom.vert", dir), TextFormat("%sgeom.frag", dir));
    shadowShader_ = LoadShader(TextFormat("%sshadow.vert", dir), TextFormat("%sshadow.frag", dir));
    outlineShader_ = LoadShader(0, TextFormat("%soutline.frag", dir));

    float shadowTexel = 1.0f / static_cast<float>(ShadowMapSize);
    SetShaderValue(toonShader_, GetShaderLocation(toonShader_, "shadowTexel"), &shadowTexel, SHADER_UNIFORM_FLOAT);
    sunDirLoc_ = GetShaderLocation(toonShader_, "sunDir");
    ambientLoc_ = GetShaderLocation(toonShader_, "ambient");
    bandsLoc_ = GetShaderLocation(toonShader_, "bands");
    shadowStrengthLoc_ = GetShaderLocation(toonShader_, "shadowStrength");
    softnessLoc_ = GetShaderLocation(toonShader_, "shadowSoftness");
    biasSlopeLoc_ = GetShaderLocation(toonShader_, "biasSlope");
    biasConstantLoc_ = GetShaderLocation(toonShader_, "biasConstant");
    shadowMapLoc_ = GetShaderLocation(toonShader_, "shadowMap");
    lightViewProjLoc_ = GetShaderLocation(toonShader_, "lightViewProj");

    float maxDepth = 60.0f;
    SetShaderValue(geomShader_, GetShaderLocation(geomShader_, "maxDepth"), &maxDepth, SHADER_UNIFORM_FLOAT);

    int width = GetScreenWidth();
    int height = GetScreenHeight();
    Vector2 texelSize = {1.0f / static_cast<float>(width), 1.0f / static_cast<float>(height)};
    float outlineWidth = 1.0f;
    float creaseCos = 0.5f;
    float depthThreshold = 0.02f;
    Vector4 outlineColor = {0.05f, 0.05f, 0.06f, 1.0f};
    SetShaderValue(outlineShader_, GetShaderLocation(outlineShader_, "texelSize"), &texelSize, SHADER_UNIFORM_VEC2);
    SetShaderValue(outlineShader_, GetShaderLocation(outlineShader_, "outlineWidth"), &outlineWidth,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, GetShaderLocation(outlineShader_, "creaseCos"), &creaseCos,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, GetShaderLocation(outlineShader_, "depthThreshold"), &depthThreshold,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, GetShaderLocation(outlineShader_, "outlineColor"), &outlineColor,
                   SHADER_UNIFORM_VEC4);
    outlineNormalDepthLoc_ = GetShaderLocation(outlineShader_, "normalDepthTex");
    screenRightLoc_ = GetShaderLocation(outlineShader_, "screenRight");
    screenUpLoc_ = GetShaderLocation(outlineShader_, "screenUp");
    cavityRadiusLoc_ = GetShaderLocation(outlineShader_, "cavityRadius");
    cavityValleyLoc_ = GetShaderLocation(outlineShader_, "cavityValley");
    cavityRidgeLoc_ = GetShaderLocation(outlineShader_, "cavityRidge");

    normalDepthTarget_ = LoadRenderTexture(width, height);
    colorTarget_ = LoadRenderTexture(width, height);
    shadowTarget_ = LoadShadowmap(ShadowMapSize, ShadowMapSize);

    water_.Load(dir, models_);

    float minX = 1e9f, maxX = -1e9f, minZ = 1e9f, maxZ = -1e9f;
    for (int r = 0; r < logic::MapRows; r++)
    {
        for (int c = 0; c < logic::MapCols; c++)
        {
            Vector3 w = CellWorld(c, r, 0.0f);
            minX = fminf(minX, w.x);
            maxX = fmaxf(maxX, w.x);
            minZ = fminf(minZ, w.z);
            maxZ = fmaxf(maxZ, w.z);
        }
    }
    Vector3 mid = {(minX + maxX) * 0.5f, 0.0f, (minZ + maxZ) * 0.5f};
    float radiusXZ = 0.5f * sqrtf((maxX - minX) * (maxX - minX) + (maxZ - minZ) * (maxZ - minZ));
    float sceneRadius = radiusXZ + 3.0f;

    lightTarget_ = mid;
    lightDist_ = sceneRadius * 2.2f;
    lightCamera_.position = Vector3Add(mid, Vector3Scale(Vector3Normalize(shadowParams_.sunDir), lightDist_));
    lightCamera_.target = mid;
    lightCamera_.up = Vector3{0.0f, 0.0f, 1.0f};
    lightCamera_.fovy = sceneRadius * 2.0f;
    lightCamera_.projection = CAMERA_ORTHOGRAPHIC;

    float ext = sceneRadius * 1.05f;
    float nearZ = lightDist_ - sceneRadius - 2.0f;
    float farZ = lightDist_ + sceneRadius + 2.0f;
    lightProj_ = MatrixOrtho(-ext, ext, -ext, ext, nearZ, farZ);
}

void Renderer::Shutdown()
{
    UnloadRenderTexture(shadowTarget_);
    UnloadRenderTexture(colorTarget_);
    UnloadRenderTexture(normalDepthTarget_);
    water_.Unload();
    UnloadShader(outlineShader_);
    UnloadShader(shadowShader_);
    UnloadShader(geomShader_);
    UnloadShader(toonShader_);
    models_.Unload();
}

void Renderer::Draw(const logic::GameState& previous, const logic::GameState& current, float alpha, Camera3D camera,
                    const logic::Map& map, const std::function<void()>& overlay)
{
    (void)previous;
    (void)current;
    (void)alpha;

    water_.Update(static_cast<float>(GetTime()));

    lightCamera_.position = Vector3Add(lightTarget_, Vector3Scale(Vector3Normalize(shadowParams_.sunDir), lightDist_));
    SetShaderValue(toonShader_, sunDirLoc_, &shadowParams_.sunDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(toonShader_, ambientLoc_, &shadowParams_.ambient, SHADER_UNIFORM_FLOAT);
    SetShaderValue(toonShader_, bandsLoc_, &shadowParams_.bands, SHADER_UNIFORM_FLOAT);
    SetShaderValue(toonShader_, shadowStrengthLoc_, &shadowParams_.shadowStrength, SHADER_UNIFORM_FLOAT);
    SetShaderValue(toonShader_, softnessLoc_, &shadowParams_.softness, SHADER_UNIFORM_FLOAT);
    SetShaderValue(toonShader_, biasSlopeLoc_, &shadowParams_.biasSlope, SHADER_UNIFORM_FLOAT);
    SetShaderValue(toonShader_, biasConstantLoc_, &shadowParams_.biasConstant, SHADER_UNIFORM_FLOAT);

    Vector3 camForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 camRight = Vector3Normalize(Vector3CrossProduct(camForward, camera.up));
    Vector3 camUp = Vector3Normalize(Vector3CrossProduct(camRight, camForward));
    float cavityValley = cavityParams_.enabled ? cavityParams_.valley : 0.0f;
    float cavityRidge = cavityParams_.enabled ? cavityParams_.ridge : 0.0f;
    SetShaderValue(outlineShader_, screenRightLoc_, &camRight, SHADER_UNIFORM_VEC3);
    SetShaderValue(outlineShader_, screenUpLoc_, &camUp, SHADER_UNIFORM_VEC3);
    SetShaderValue(outlineShader_, cavityRadiusLoc_, &cavityParams_.radius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, cavityValleyLoc_, &cavityValley, SHADER_UNIFORM_FLOAT);
    SetShaderValue(outlineShader_, cavityRidgeLoc_, &cavityRidge, SHADER_UNIFORM_FLOAT);

    if (IsKeyPressed(KEY_F2)) debugShadow_ = !debugShadow_;

    Scene scene(models_);

    models_.ApplyShader(shadowShader_);
    BeginTextureMode(shadowTarget_);
    ClearBackground(WHITE);
    BeginMode3D(lightCamera_);
    rlSetMatrixProjection(lightProj_);
    Matrix lightView = rlGetMatrixModelview();
    scene.DrawFloors(map);
    scene.DrawStructures(map);
    EndMode3D();
    EndTextureMode();

    Matrix lightViewProj = MatrixMultiply(lightView, lightProj_);
    SetShaderValueMatrix(toonShader_, lightViewProjLoc_, lightViewProj);

    models_.ApplyShader(geomShader_);
    BeginTextureMode(normalDepthTarget_);
    ClearBackground(BLANK);
    rlDisableColorBlend();
    BeginMode3D(camera);
    scene.DrawStructures(map);
    EndMode3D();
    rlEnableColorBlend();
    EndTextureMode();

    models_.ApplyShader(toonShader_);
    BeginTextureMode(colorTarget_);
    ClearBackground(Color{26, 26, 28, 255});
    int shadowSlot = ShadowSlot;
    BeginMode3D(camera);
    rlEnableShader(toonShader_.id);
    rlActiveTextureSlot(shadowSlot);
    rlEnableTexture(shadowTarget_.depth.id);
    rlSetUniform(shadowMapLoc_, &shadowSlot, SHADER_UNIFORM_INT, 1);
    scene.DrawFloors(map);
    scene.DrawStructures(map);
    water_.Draw(models_);
    EndMode3D();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    BeginShaderMode(outlineShader_);
    SetShaderValueTexture(outlineShader_, outlineNormalDepthLoc_, normalDepthTarget_.texture);
    DrawTextureRec(
        colorTarget_.texture,
        Rectangle{
            0.0f, 0.0f, static_cast<float>(colorTarget_.texture.width),
            -static_cast<float>(colorTarget_.texture.height)
        },
        Vector2{0.0f, 0.0f},
        WHITE);
    EndShaderMode();
    if (debugShadow_)
    {
        float size = 260.0f;
        float x = static_cast<float>(GetScreenWidth()) - size - 10.0f;
        DrawTexturePro(
            shadowTarget_.depth,
            Rectangle{0.0f, 0.0f, static_cast<float>(ShadowMapSize), static_cast<float>(ShadowMapSize)},
            Rectangle{x, 10.0f, size, size}, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        DrawRectangleLines(static_cast<int>(x), 10, static_cast<int>(size), static_cast<int>(size), GREEN);
        DrawText("shadow map (F2)", static_cast<int>(x), static_cast<int>(size) + 14, 10, GREEN);
    }
    DrawFPS(10, 10);
    if (overlay) overlay();
    EndDrawing();
}
}
