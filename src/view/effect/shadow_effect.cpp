#include "view/effect/shadow_effect.h"

#include <cmath>

#include "raymath.h"
#include "rlgl.h"

#include "data/render/render_params.h"
#include "logic/world/map.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/scene.h"
#include "view/render/render_targets.h"
#include "view/space/world_space.h"

namespace view
{
void ShadowEffect::Init(Shader shader)
{
    shader_ = shader;
    target_ = LoadShadowmap(data::Render.shadowMapSize, data::Render.shadowMapSize);

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
    float sceneRadius = radiusXZ + data::Render.lightScenePad;

    lightTarget_ = mid;
    lightDist_ = sceneRadius * data::Render.lightDistMul;
    lightCamera_.target = mid;
    lightCamera_.up = Vector3{0.0f, 0.0f, 1.0f};
    lightCamera_.fovy = sceneRadius * data::Render.lightFovMul;
    lightCamera_.projection = CAMERA_ORTHOGRAPHIC;

    float ext = sceneRadius * data::Render.lightExtMul;
    float nearZ = lightDist_ - sceneRadius - data::Render.lightNearFarPad;
    float farZ = lightDist_ + sceneRadius + data::Render.lightNearFarPad;
    lightProj_ = MatrixOrtho(-ext, ext, -ext, ext, nearZ, farZ);
}

void ShadowEffect::Shutdown()
{
    UnloadRenderTexture(target_);
}

void ShadowEffect::RenderMap(ModelRegistry& models, Vector3 sunDir, const std::function<void()>& drawScene)
{
    lightCamera_.position = Vector3Add(lightTarget_, Vector3Scale(Vector3Normalize(sunDir), lightDist_));
    models.ApplyShader(shader_);
    BeginTextureMode(target_);
    ClearBackground(WHITE);
    BeginMode3D(lightCamera_);
    rlSetMatrixProjection(lightProj_);
    Matrix lightView = rlGetMatrixModelview();
    if (drawScene) drawScene();
    EndMode3D();
    EndTextureMode();
    lightViewProj_ = MatrixMultiply(lightView, lightProj_);
}

void ShadowEffect::DrawDebugPreview() const
{
    float size = data::Render.shadowPreviewSize;
    float margin = data::Render.shadowPreviewMargin;
    float x = static_cast<float>(GetScreenWidth()) - size - margin;
    DrawTexturePro(
        target_.depth,
        Rectangle{0.0f, 0.0f, static_cast<float>(data::Render.shadowMapSize),
                  static_cast<float>(data::Render.shadowMapSize)},
        Rectangle{x, margin, size, size}, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
    DrawRectangleLines(static_cast<int>(x), static_cast<int>(margin), static_cast<int>(size),
                       static_cast<int>(size), GREEN);
    DrawText("shadow map (F2)", static_cast<int>(x), static_cast<int>(size) + 14, 10, GREEN);
}
}
