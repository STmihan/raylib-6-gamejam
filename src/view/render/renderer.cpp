#include "view/render/renderer.h"

#include "data/render/render_params.h"
#include "view/prefab/scene.h"

namespace view
{
void Renderer::Init()
{
    models_.Load();
    shaders_.Load();

    shadow_.Init(shaders_.Shadow());
    toon_.Init(shaders_.Toon());
    outline_.Init(shaders_.Geom(), shaders_.Outline());
    water_.Load(shaders_.Water(), models_);

    colorTarget_ = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
}

void Renderer::Shutdown()
{
    UnloadRenderTexture(colorTarget_);
    outline_.Shutdown();
    shadow_.Shutdown();
    water_.Unload();
    shaders_.Unload();
    models_.Unload();
}

void Renderer::Draw(const logic::GameState& previous, const logic::GameState& current, float alpha, Camera3D camera,
                    const logic::Map& map, const std::function<void()>& overlay)
{
    (void)previous;
    (void)current;
    (void)alpha;

    if (IsKeyPressed(KEY_F2)) debugShadow_ = !debugShadow_;

    water_.Update(static_cast<float>(GetTime()));
    Scene scene(models_);

    shadow_.RenderMap(models_, scene, map, shadowParams_.sunDir);
    outline_.RenderNormalDepth(models_, scene, map, camera);

    toon_.Apply(models_);
    BeginTextureMode(colorTarget_);
    ClearBackground(data::Render.backgroundColor);
    BeginMode3D(camera);
    toon_.Upload(shadowParams_, shadow_.LightViewProj());
    toon_.BindShadowMap(shadow_.DepthTextureId(), shadow_.Slot());
    scene.Draw(map, true);
    water_.Draw(models_);
    EndMode3D();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    outline_.Composite(colorTarget_, cavityParams_, camera);
    if (debugShadow_) shadow_.DrawDebugPreview();
    DrawFPS(10, 10);
    if (overlay) overlay();
    EndDrawing();
}
}
