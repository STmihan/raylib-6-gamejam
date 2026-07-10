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
    outline_.Init(shaders_.Geom(), shaders_.Outline(), shaders_.Mask());
    water_.Load(shaders_.Water(), shaders_.WaterLine(), models_);

    colorTarget_ = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    hpBars_.Load();
    muzzles_.Load();
}

void Renderer::Shutdown()
{
    UnloadRenderTexture(colorTarget_);
    hpBars_.Unload();
    hexGrid_.Unload();
    outline_.Shutdown();
    shadow_.Shutdown();
    water_.Unload();
    shaders_.Unload();
    models_.Unload();
}

void Renderer::UseEnvShadow()
{
    toon_.Upload(shadowParams_, shadow_.LightViewProj());
}

void Renderer::UseUnitShadow()
{
    unitShadowParams_.sunDir = shadowParams_.sunDir;
    toon_.Upload(unitShadowParams_, shadow_.LightViewProj());
}

void Renderer::RenderPasses(Camera3D camera, const ScenePasses& passes, const std::function<void()>& overlay)
{
    shadow_.RenderMap(models_, shadowParams_.sunDir, passes.shadow);
    outline_.RenderNormalDepth(models_, camera, passes.geom);
    outline_.RenderUnitMask(models_, camera, passes.mask);

    toon_.Apply(models_);
    BeginTextureMode(colorTarget_);
    ClearBackground(data::Render.backgroundColor);
    BeginMode3D(camera);
    toon_.BindShadowMap(shadow_.DepthTextureId(), shadow_.Slot());
    passes.color();
    EndMode3D();
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    outline_.Composite(colorTarget_, cavityParams_, camera);
    if (passes.composite2D) passes.composite2D();
    if (overlay) overlay();
    EndDrawing();
}

void Renderer::Draw(const logic::GameState& previous, const logic::GameState& current, float alpha, float animTime,
                    Camera3D camera, const logic::Map& map, const std::function<void()>& overlay)
{
    if (preview_.Active())
    {
        DrawPreview(overlay);
        return;
    }
    if (airTest_.Active())
    {
        DrawAirTest(overlay);
        return;
    }
    if (projTest_.Active())
    {
        DrawProjectileTest(overlay);
        return;
    }

    if (!hexGridLoaded_)
    {
        hexGrid_.Load(map);
        hexGridLoaded_ = true;
    }

    anim_.Update(previous, current);
    units_.UpdateFlash(current, GetFrameTime());
    water_.Update(animTime);
    Scene scene(models_);

    auto drawUnits = [&] {
        units_.Draw(models_, previous, current, alpha, false, orbitParams_, animTime);
        projectiles_.Draw(models_, muzzles_, previous, current, alpha, orbitParams_, animTime);
    };

    ScenePasses passes;
    passes.shadow = [&] { scene.Draw(map, true); drawUnits(); };
    passes.geom = [&] { scene.Draw(map, false); drawUnits(); };
    passes.mask = drawUnits;
    passes.color = [&] {
        UseEnvShadow();
        scene.Draw(map, true);
        hexGrid_.Draw();
        UseUnitShadow();
        units_.Draw(models_, previous, current, alpha, true, orbitParams_, animTime);
        projectiles_.Draw(models_, muzzles_, previous, current, alpha, orbitParams_, animTime);
        UseEnvShadow();
        water_.Draw(models_);
    };
    passes.composite2D = [&] {
        hpBars_.DrawEntities(camera, previous, current, alpha);
        if (current.winner >= 0)
        {
            const char* text = current.winner == 0 ? "TOP WINS" : "BOTTOM WINS";
            int fontSize = 60;
            int width = MeasureText(text, fontSize);
            DrawText(text, (GetScreenWidth() - width) / 2, GetScreenHeight() / 2 - fontSize / 2, fontSize, RAYWHITE);
        }
#if defined(DEBUG_BUILD)
        DrawFPS(10, 10);
#endif
    };

    RenderPasses(camera, passes, overlay);
}

void Renderer::DrawPreview(const std::function<void()>& overlay)
{
    preview_.UpdateCamera();
    preview_.UpdateFlash(GetFrameTime());
    Camera3D camera = preview_.Camera();

    auto drawItem = [&] { preview_.Draw(models_); };

    ScenePasses passes;
    passes.shadow = drawItem;
    passes.geom = drawItem;
    passes.mask = [] {};
    passes.color = [&] {
        UseUnitShadow();
        DrawGrid(20, 1.0f);
        preview_.DrawGizmo();
        preview_.Draw(models_);
    };
    passes.composite2D = [&] {
        hpBars_.DrawSingle(camera, Vector3{0.0f, 1.8f, 0.0f}, preview_.HpFraction(), Color{90, 200, 90, 255});
    };

    RenderPasses(camera, passes, overlay);
}

void Renderer::DrawAirTest(const std::function<void()>& overlay)
{
    airTest_.UpdateCamera();
    Camera3D camera = airTest_.Camera();
    const logic::GameState& state = airTest_.State();
    units_.UpdateFlash(state, GetFrameTime());
    float time = static_cast<float>(GetTime());

    auto drawUnits = [&] {
        units_.Draw(models_, state, state, 0.0f, false, orbitParams_, time);
        projectiles_.Draw(models_, muzzles_, state, state, 0.0f, orbitParams_, time);
    };

    ScenePasses passes;
    passes.shadow = drawUnits;
    passes.geom = drawUnits;
    passes.mask = drawUnits;
    passes.color = [&] {
        UseEnvShadow();
        DrawGrid(20, 1.0f);
        units_.Draw(models_, state, state, 0.0f, true, orbitParams_, time);
        projectiles_.Draw(models_, muzzles_, state, state, 0.0f, orbitParams_, time);
    };
    passes.composite2D = [&] { hpBars_.DrawEntities(camera, state, state, 0.0f); };

    RenderPasses(camera, passes, overlay);
}

void Renderer::DrawProjectileTest(const std::function<void()>& overlay)
{
    projTest_.UpdateCamera();
    projTest_.Update(GetFrameTime());
    Camera3D camera = projTest_.Camera();
    const logic::GameState& prev = projTest_.Previous();
    const logic::GameState& cur = projTest_.Current();
    float alpha = projTest_.Alpha();
    units_.UpdateFlash(cur, GetFrameTime());
    float time = static_cast<float>(GetTime());

    auto drawUnits = [&] {
        units_.Draw(models_, prev, cur, alpha, false, orbitParams_, time);
        projectiles_.Draw(models_, muzzles_, prev, cur, alpha, orbitParams_, time);
    };

    ScenePasses passes;
    passes.shadow = drawUnits;
    passes.geom = drawUnits;
    passes.mask = drawUnits;
    passes.color = [&] {
        UseEnvShadow();
        DrawGrid(20, 1.0f);
        units_.Draw(models_, prev, cur, alpha, true, orbitParams_, time);
        projectiles_.Draw(models_, muzzles_, prev, cur, alpha, orbitParams_, time);
        projectiles_.DrawMuzzleGizmos(models_, muzzles_, prev, cur, alpha, orbitParams_);
    };
    passes.composite2D = [&] { hpBars_.DrawEntities(camera, prev, cur, alpha); };

    RenderPasses(camera, passes, overlay);
}
}
