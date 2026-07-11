#include "view/render/renderer.h"

#include <cmath>

#include "rlgl.h"

#include "data/card/card.h"
#include "data/render/render_params.h"
#include "data/space/hex.h"
#include "data/space/world_config.h"
#include "data/tile/tile.h"
#include "logic/deploy.h"
#include "view/prefab/scene.h"
#include "view/space/hex_decal.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    bool Deployable(const logic::Map& map, int col, int row, data::UnitType type)
    {
        return logic::IsDeployable(map, col, row, data::PlayerTeam, type);
    }

    void DrawCellOutline(Vector3 center, const logic::Map& map, int col, int row, bool boundaryOnly,
                         float width, Color color, data::UnitType type)
    {
        for (int dir = 0; dir < 6; dir++)
        {
            if (boundaryOnly)
            {
                data::Offset nb = data::Neighbor({col, row}, dir);
                if (map.InBounds(nb.col, nb.row) && Deployable(map, nb.col, nb.row, type)) continue;
            }
            DrawHexEdge(HexCorner(center, HexEdgeCorners[dir][0]), HexCorner(center, HexEdgeCorners[dir][1]), width,
                        color);
        }
    }
}

void Renderer::Init()
{
    models_.Load();
    shaders_.Load();
    textures_.Load();

    shadow_.Init(shaders_.Shadow());
    toon_.Init(shaders_.Toon());
    outline_.Init(shaders_.Geom(), shaders_.Outline(), shaders_.Mask());
    water_.Load(shaders_.Water(), shaders_.WaterLine(), models_);

    colorTarget_ = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    cardTarget_ = LoadRenderTexture(200, 240);
    SetTextureFilter(cardTarget_.texture, TEXTURE_FILTER_BILINEAR);
    hpBars_.Load(textures_.White());
    deployRings_.Load(shaders_.Ring(), textures_.White());
    muzzles_.Load();
    ui_.Load(shaders_, textures_);
}

void Renderer::Shutdown()
{
    ui_.Unload();
    UnloadRenderTexture(cardTarget_);
    UnloadRenderTexture(colorTarget_);
    hexGrid_.Unload();
    outline_.Shutdown();
    shadow_.Shutdown();
    water_.Unload();
    textures_.Unload();
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

void Renderer::DrawDeployOverlay(Camera3D camera, const logic::Map& map, float time, bool affordable)
{
    if (!hand_.Dragging()) return;
    data::UnitType type = hand_.DraggedType();

    const Color zoneFill = {90, 170, 255, 90};
    const Color zoneLine = {110, 200, 255, 255};
    const Color whiteFill = {255, 255, 255, 200};
    const Color whiteLine = {255, 255, 255, 255};
    const Color redFill = {255, 110, 100, 200};
    const Color redLine = {255, 70, 60, 255};
    const float lineW = 0.09f;

    BeginBlendMode(BLEND_ALPHA);
    for (int row = logic::MapRows / 2; row < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (!Deployable(map, col, row, type)) continue;
            DrawModelYaw(models_.TileWhite(), LogicToWorld(data::CellToLogic(col, row), 0.06f), 0.0f, zoneFill);
        }
    }
    EndBlendMode();

    for (int row = logic::MapRows / 2; row < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (!Deployable(map, col, row, type)) continue;
            Vector3 c = LogicToWorld(data::CellToLogic(col, row), 0.10f);
            DrawCellOutline(c, map, col, row, true, lineW, zoneLine, type);
        }
    }

    Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
    if (ray.direction.y > -0.0001f) return;
    float t = -ray.position.y / ray.direction.y;
    data::Vec2 logicPos = {(ray.position.x + ray.direction.x * t) / data::RenderScale,
                           (ray.position.z + ray.direction.z * t) / data::RenderScale};
    data::Offset cell = data::CellFromLogic(logicPos);
    if (!map.InBounds(cell.col, cell.row)) return;

    bool occupied = occluded_[static_cast<std::size_t>(cell.row) * logic::MapCols + cell.col];
    bool canPlace = affordable && Deployable(map, cell.col, cell.row, type) && !occupied;
    data::Vec2 center = data::CellToLogic(cell.col, cell.row);
    BeginBlendMode(BLEND_ALPHA);
    DrawModelYaw(models_.TileWhite(), LogicToWorld(center, 0.12f), 0.0f, canPlace ? whiteFill : redFill);
    EndBlendMode();
    DrawCellOutline(LogicToWorld(center, 0.16f), map, cell.col, cell.row, false, lineW,
                    canPlace ? whiteLine : redLine, type);
    if (canPlace)
    {
        BeginBlendMode(BLEND_ALPHA);
        units_.DrawPreview(models_, type, center, data::PlayerTeam, orbitParams_, time, Color{255, 255, 255, 200});
        EndBlendMode();
    }
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

    occluded_.fill(false);
    wallAlive_.fill(false);
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& e = current.entities[i];
        if (!e.active) continue;
        if (e.col < 0 || e.col >= logic::MapCols || e.row < 0 || e.row >= logic::MapRows) continue;
        std::size_t idx = static_cast<std::size_t>(e.row) * logic::MapCols + e.col;
        if (e.kind == logic::EntityKind::Unit) occluded_[idx] = true;
        else if (e.kind == logic::EntityKind::Wall) wallAlive_[idx] = true;
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
    passes.shadow = [&] { scene.Draw(map, true, nullptr, wallAlive_.data()); drawUnits(); };
    passes.geom = [&] { scene.Draw(map, false, occluded_.data(), wallAlive_.data()); drawUnits(); };
    passes.mask = drawUnits;
    passes.color = [&] {
        UseEnvShadow();
        scene.Draw(map, true, occluded_.data(), wallAlive_.data());
        hexGrid_.Draw();
        UseUnitShadow();
        units_.Draw(models_, previous, current, alpha, true, orbitParams_, animTime);
        projectiles_.Draw(models_, muzzles_, previous, current, alpha, orbitParams_, animTime);
        healWaves_.Draw(current, alpha);
        bool affordable = true;
        if (hand_.Dragging())
        {
            int cost = data::CardDefOf(hand_.DraggedType()).cost;
            affordable = current.resource[data::TeamIndex(data::PlayerTeam)] >= static_cast<float>(cost);
        }
        DrawDeployOverlay(camera, map, animTime, affordable);
        UseEnvShadow();
        scene.DrawGhostStructures(map, occluded_.data(), wallAlive_.data(), 110);
        water_.Draw(models_);
        controlOverlay_.Draw(previous, current, alpha, animTime, map, models_, units_, orbitParams_,
                             occluded_.data());
    };
    passes.composite2D = [&] {
        hpBars_.DrawEntities(camera, previous, current, alpha);
        deployRings_.Draw(camera, previous, current, alpha);
        if (!hudHidden_)
        {
            const Color panelTint = {28, 32, 24, 235};

            Rectangle timer = {14.0f, 12.0f, 120.0f, 48.0f};
            ui::Panel(ui_, timer, panelTint);
            ui::LabelCentered(ui_, "01:45", timer, 30.0f, RAYWHITE, true);

            int player = data::TeamIndex(data::PlayerTeam);
            float res = previous.resource[player] * (1.0f - alpha) + current.resource[player] * alpha;
            int highlight = hand_.HasHighlight() ? data::CardDefOf(hand_.HighlightType()).cost
                                                 : resourceHighlight_;
            ui::Panel(ui_, Rectangle{486.0f, 604.0f, 220.0f, 40.0f}, panelTint);
            ui::DrawResourceBar(ui_, Rectangle{498.0f, 613.0f, 196.0f, 22.0f}, res, 6, highlight,
                                animTime, crystalStyle_);

            Rectangle mull = {586.0f, 652.0f, 120.0f, 46.0f};
            ui::ButtonBg(ui_, mull, panelTint);
            ui::Icon(ui_, ui_.Theme().Cards(), Rectangle{mull.x + 12.0f, mull.y + 7.0f, 32.0f, 32.0f},
                     Color{234, 230, 214, 255});
            ui::Label(ui_, "2", Vector2{mull.x + 66.0f, mull.y + 10.0f}, 26.0f,
                      Color{240, 236, 221, 255}, true);

            hand_.Draw(ui_, textures_, cardTarget_);
        }
        if (current.winner >= 0)
        {
            const char* text = current.winner == 0 ? "TOP WINS" : "BOTTOM WINS";
            int fontSize = 60;
            int width = MeasureText(text, fontSize);
            DrawText(text, (GetScreenWidth() - width) / 2, GetScreenHeight() / 2 - fontSize / 2, fontSize, RAYWHITE);
            const char* sub = "Press R to restart";
            int subSize = 26;
            int subWidth = MeasureText(sub, subSize);
            DrawText(sub, (GetScreenWidth() - subWidth) / 2, GetScreenHeight() / 2 + fontSize / 2 + 12, subSize,
                     Color{210, 210, 210, 255});
        }
#if defined(DEBUG_BUILD)
        DrawFPS(18, 70);
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
