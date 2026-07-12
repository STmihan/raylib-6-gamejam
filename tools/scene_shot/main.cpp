#include <cmath>
#include <cstring>
#include <vector>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include "data/render/render_params.h"
#include "data/render/shadow_params.h"
#include "data/scene/camera_config.h"
#include "data/space/hex.h"
#include "data/space/world_config.h"
#include "view/effect/shadow_effect.h"
#include "view/effect/toon_effect.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/registries/shader_registry.h"
#include "view/space/world_space.h"

namespace
{
constexpr int Cols = 100;
constexpr int Rows = 100;
constexpr float CameraHeight = 42.0f;
const char* OutputSquare = "C:/lab-ssd/raylib-6-gamejam/tiles_2048.png";
const char* OutputWide = "C:/lab-ssd/raylib-6-gamejam/tiles_1920x1080.png";

constexpr float GridOuterRadius = 0.98f;
constexpr float GridThickness = 0.055f;
constexpr float GridY = 0.03f;

void GridCorner(Vector3 center, float radius, int k, float* out)
{
    float angle = (60.0f * static_cast<float>(k) - 30.0f) * 0.01745329f;
    out[0] = center.x + radius * std::cos(angle);
    out[1] = GridY;
    out[2] = center.z + radius * std::sin(angle);
}

Mesh BuildGridMesh()
{
    float innerRadius = GridOuterRadius - GridThickness;
    int vertexCount = Cols * Rows * 6 * 6;
    std::vector<float> vertices(static_cast<std::size_t>(vertexCount) * 3, 0.0f);
    std::vector<float> normals(static_cast<std::size_t>(vertexCount) * 3, 0.0f);
    std::vector<unsigned char> colors(static_cast<std::size_t>(vertexCount) * 4, 255);

    int v = 0;
    auto push = [&](const float* p) {
        vertices[static_cast<std::size_t>(v) * 3 + 0] = p[0];
        vertices[static_cast<std::size_t>(v) * 3 + 1] = p[1];
        vertices[static_cast<std::size_t>(v) * 3 + 2] = p[2];
        normals[static_cast<std::size_t>(v) * 3 + 1] = 1.0f;
        v++;
    };

    for (int row = 0; row < Rows; row++)
    {
        for (int col = 0; col < Cols; col++)
        {
            Vector3 center = view::CellWorld(col, row, 0.0f);
            for (int k = 0; k < 6; k++)
            {
                float oa[3], ob[3], ia[3], ib[3];
                GridCorner(center, GridOuterRadius, k, oa);
                GridCorner(center, GridOuterRadius, (k + 1) % 6, ob);
                GridCorner(center, innerRadius, k, ia);
                GridCorner(center, innerRadius, (k + 1) % 6, ib);
                push(oa); push(ob); push(ib);
                push(oa); push(ib); push(ia);
            }
        }
    }

    Mesh mesh = {};
    mesh.vertexCount = vertexCount;
    mesh.triangleCount = vertexCount / 3;
    mesh.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(vertexCount) * 3 * sizeof(float)));
    mesh.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(vertexCount) * 3 * sizeof(float)));
    mesh.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(vertexCount) * 4));
    std::memcpy(mesh.vertices, vertices.data(), vertices.size() * sizeof(float));
    std::memcpy(mesh.normals, normals.data(), normals.size() * sizeof(float));
    std::memcpy(mesh.colors, colors.data(), colors.size());
    UploadMesh(&mesh, false);
    return mesh;
}
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_HIDDEN);
    InitWindow(1280, 720, "scene_shot");
    ChangeDirectory(GetApplicationDirectory());

    view::ShaderRegistry shaders;
    shaders.Load();
    view::ModelRegistry models;
    models.Load();
    view::ShadowEffect shadow;
    shadow.Init(shaders.Shadow());
    view::ToonEffect toon;
    toon.Init(shaders.Toon());

    data::ShadowParams params;
    params.shadowStrength = 0.0f;

    Mesh grid = BuildGridMesh();
    Material gridMaterial = LoadMaterialDefault();
    gridMaterial.maps[MATERIAL_MAP_DIFFUSE].color = Color{0, 0, 0, static_cast<unsigned char>(0.249f * 255.0f)};

    data::Vec2 firstCell = data::CellToLogic(0, 0);
    data::Vec2 lastCell = data::CellToLogic(Cols - 1, Rows - 1);
    data::Vec2 centerLogic = {(firstCell.x + lastCell.x) * 0.5f, (firstCell.y + lastCell.y) * 0.5f};
    Vector3 center = view::LogicToWorld(centerLogic, 0.0f);

    Camera3D camera = {};
    camera.position = {center.x, CameraHeight, center.z};
    camera.target = center;
    camera.up = {0.0f, 0.0f, -1.0f};
    camera.fovy = data::CameraFovy;
    camera.projection = CAMERA_PERSPECTIVE;

    auto drawTiles = [&]() {
        for (int row = 0; row < Rows; row++)
        {
            for (int col = 0; col < Cols; col++)
            {
                view::DrawModelYaw(models.TileWhite(), view::CellWorld(col, row, 0.0f), 0.0f, WHITE);
            }
        }
    };

    shadow.RenderMap(models, params.sunDir, drawTiles);
    toon.Apply(models);
    toon.Upload(params, shadow.LightViewProj());

    auto renderAndExport = [&](int width, int height, const char* path) {
        RenderTexture2D output = LoadRenderTexture(width, height);
        BeginTextureMode(output);
        ClearBackground(data::Render.backgroundColor);
        BeginMode3D(camera);
        toon.BindShadowMap(shadow.DepthTextureId(), shadow.Slot());
        drawTiles();
        BeginBlendMode(BLEND_ALPHA);
        rlDisableBackfaceCulling();
        DrawMesh(grid, gridMaterial, MatrixIdentity());
        rlEnableBackfaceCulling();
        EndBlendMode();
        EndMode3D();
        EndTextureMode();

        Image shot = LoadImageFromTexture(output.texture);
        ImageFlipVertical(&shot);
        ExportImage(shot, path);
        TraceLog(LOG_WARNING, "scene_shot: saved %s", path);
        UnloadImage(shot);
        UnloadRenderTexture(output);
    };

    renderAndExport(2048, 2048, OutputSquare);
    renderAndExport(1920, 1080, OutputWide);

    UnloadMesh(grid);
    shadow.Shutdown();
    shaders.Unload();
    models.Unload();
    CloseWindow();
    return 0;
}
