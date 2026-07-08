#include "view/renderer.h"

#include <cmath>
#include <vector>

#include "raymath.h"
#include "rlgl.h"

#include "data/hex.h"
#include "data/tile.h"
#include "data/vec.h"
#include "data/world_config.h"
#include "logic/map.h"

namespace view
{
namespace
{
    constexpr float RadToDeg = 57.2957795f;
    constexpr float WaterHeight = -0.1f;
    constexpr float ShadowOrthoSize = 56.0f;
    constexpr int ShadowMapSize = 2048;
    constexpr int ShadowSlot = 10;
    constexpr int SdfSlot = 11;
    constexpr float SdfMaxDist = 16.0f;
    constexpr int SdfResolution = 1024;
    constexpr float HexCircumradius = 1.0f;

    Vector3 LogicToWorld(data::Vec2 logic, float height)
    {
        return {logic.x * data::RenderScale, height, logic.y * data::RenderScale};
    }

    Vector3 CellWorld(int col, int row, float height)
    {
        return LogicToWorld(data::CellToLogic(col, row), height);
    }

    Vector3 Midpoint(Vector3 a, Vector3 b)
    {
        return {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f, (a.z + b.z) * 0.5f};
    }

    Vector3 SceneCenterWorld()
    {
        data::Vec2 center = data::FieldCenterLogic();
        return {center.x * data::RenderScale, 0.0f, center.y * data::RenderScale};
    }

    void DrawModelYaw(const Model& model, Vector3 position, float yawDegrees, Color tint)
    {
        DrawModelEx(model, position, Vector3{0.0f, 1.0f, 0.0f}, yawDegrees, Vector3{1.0f, 1.0f, 1.0f}, tint);
    }

    float SwampEdgeYaw(const logic::Map& map, int col, int row)
    {
        Vector3 here = CellWorld(col, row, 0.0f);
        for (int dir = 0; dir < 6; dir++)
        {
            data::Offset n = data::Neighbor({col, row}, dir);
            if (!map.InBounds(n.col, n.row)) continue;
            if (map.At(n.col, n.row) != data::TileType::SwampCenter) continue;
            Vector3 there = CellWorld(n.col, n.row, 0.0f);
            return std::atan2(-(there.z - here.z), there.x - here.x) * RadToDeg;
        }
        return 0.0f;
    }

    void DrawWater(const ModelRegistry& models)
    {
        Vector3 center = SceneCenterWorld();
        DrawModel(models.Water(), Vector3{center.x, WaterHeight, center.z}, 1.0f, WHITE);
    }

    void DrawFloors(const ModelRegistry& models, const logic::Map& map)
    {
        for (int row = 0; row < logic::MapRows; row++)
        {
            for (int col = 0; col < logic::MapCols; col++)
            {
                data::TileType type = map.At(col, row);
                bool facesSwamp = type == data::TileType::SwampEdge || type == data::TileType::SwampCorner;
                float yaw = facesSwamp ? SwampEdgeYaw(map, col, row) : 0.0f;
                DrawModelYaw(models.FloorFor(type), CellWorld(col, row, 0.0f), yaw, WHITE);
            }
        }
    }

    void DrawWalls(const ModelRegistry& models, const logic::Map& map)
    {
        for (int row = 0; row < logic::MapRows; row++)
        {
            for (int col = 0; col < logic::MapCols; col++)
            {
                if (map.At(col, row) == data::TileType::Wall)
                {
                    DrawModelYaw(models.Wall(), CellWorld(col, row, 0.0f), 0.0f, WHITE);
                }
            }
        }
    }

    void DrawTrees(const ModelRegistry& models, const logic::Map& map)
    {
        const Vector3 offsets[3] = {
            {0.42f, 0.0f, 0.24f},
            {-0.42f, 0.0f, 0.24f},
            {0.0f, 0.0f, -0.42f},
        };
        for (int row = 0; row < logic::MapRows; row++)
        {
            for (int col = 0; col < logic::MapCols; col++)
            {
                if (map.At(col, row) != data::TileType::Forest) continue;
                Vector3 center = CellWorld(col, row, 0.0f);
                for (int k = 0; k < 3; k++)
                {
                    Vector3 position = {center.x + offsets[k].x, center.y, center.z + offsets[k].z};
                    int treeIndex = (col * 7 + row * 13 + k) % 3;
                    float yaw = static_cast<float>(((col + row + k) * 57) % 360);
                    DrawModelYaw(models.Tree(treeIndex), position, yaw, WHITE);
                }
            }
        }
    }

    void DrawBases(const ModelRegistry& models, const logic::Map& map)
    {
        for (int row = 0; row + 1 < logic::MapRows; row++)
        {
            for (int col = 0; col < logic::MapCols; col++)
            {
                bool isTopOfPair = map.At(col, row) == data::TileType::Base
                    && map.At(col, row + 1) == data::TileType::Base
                    && (row == 0 || map.At(col, row - 1) != data::TileType::Base);
                if (!isTopOfPair) continue;

                Vector3 mid = Midpoint(CellWorld(col, row, 0.0f), CellWorld(col, row + 1, 0.0f));
                float yaw = (row >= logic::MapRows / 2 ? 180.0f : 0.0f) + (col % 2 == 0 ? 180.0f : 0.0f);
                DrawModelYaw(models.BaseSection(), mid, yaw, WHITE);
            }
        }
    }

    void DrawLand(const ModelRegistry& models, const logic::Map& map)
    {
        DrawFloors(models, map);
        DrawWalls(models, map);
        DrawTrees(models, map);
        DrawBases(models, map);
    }

    void DrawObjects(const ModelRegistry& models, const logic::Map& map)
    {
        DrawWalls(models, map);
        DrawTrees(models, map);
        DrawBases(models, map);
    }

    const char* ShaderDir()
    {
#if defined(__EMSCRIPTEN__)
        return "assets/shaders/glsl100/";
#else
        return "assets/shaders/glsl330/";
#endif
    }

    void SetWaterUniforms(Shader shader, const WaterParams& w)
    {
        SetShaderValue(shader, GetShaderLocation(shader, "deepColor"), &w.deep, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, GetShaderLocation(shader, "shallowColor"), &w.shallow, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, GetShaderLocation(shader, "foamColor"), &w.foam, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, GetShaderLocation(shader, "outlineColor"), &w.outline, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, GetShaderLocation(shader, "colorRange"), &w.colorRange, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "foamDistance"), &w.foamDistance, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "foamCutoff"), &w.foamCutoff, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "noiseScale"), &w.noiseScale, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "distortAmount"), &w.distortAmount, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "scrollSpeed"), &w.scrollSpeed, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "outlineWidth"), &w.outlineWidth, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "flowSpeed"), &w.flowSpeed, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, GetShaderLocation(shader, "flowAmount"), &w.flowAmount, SHADER_UNIFORM_FLOAT);
    }

    RenderTexture2D LoadShadowmap(int width, int height)
    {
        RenderTexture2D target = {0};
        target.id = rlLoadFramebuffer();
        target.texture.width = width;
        target.texture.height = height;
        if (target.id > 0)
        {
            rlEnableFramebuffer(target.id);
            target.depth.id = rlLoadTextureDepth(width, height, false);
            target.depth.width = width;
            target.depth.height = height;
            target.depth.format = 19;
            target.depth.mipmaps = 1;
            rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
            rlFramebufferComplete(target.id);
            rlDisableFramebuffer();
        }
        return target;
    }

    void RoundToCell(float q, float r, int& col, int& row)
    {
        float x = q;
        float z = r;
        float y = -x - z;
        float rx = std::round(x);
        float ry = std::round(y);
        float rz = std::round(z);
        float dx = std::fabs(rx - x);
        float dy = std::fabs(ry - y);
        float dz = std::fabs(rz - z);
        if (dx > dy && dx > dz)
        {
            rx = -ry - rz;
        }
        else if (dy > dz)
        {
            ry = -rx - rz;
        }
        else
        {
            rz = -rx - ry;
        }
        int cubeX = static_cast<int>(rx);
        int cubeZ = static_cast<int>(rz);
        col = cubeX + (cubeZ - (cubeZ & 1)) / 2;
        row = cubeZ;
    }

    void SquaredDistance1D(const std::vector<float>& f, std::vector<float>& d, std::vector<int>& v,
                           std::vector<float>& z, int n)
    {
        const float inf = 1e18f;
        int k = 0;
        v[0] = 0;
        z[0] = -inf;
        z[1] = inf;
        for (int q = 1; q < n; q++)
        {
            float s = ((f[q] + static_cast<float>(q * q)) - (f[v[k]] + static_cast<float>(v[k] * v[k]))) /
                static_cast<float>(2 * q - 2 * v[k]);
            while (s <= z[k])
            {
                k--;
                s = ((f[q] + static_cast<float>(q * q)) - (f[v[k]] + static_cast<float>(v[k] * v[k]))) / static_cast
                    <float>(2 * q - 2 * v[k]);
            }
            k++;
            v[k] = q;
            z[k] = s;
            z[k + 1] = inf;
        }
        k = 0;
        for (int q = 0; q < n; q++)
        {
            while (z[k + 1] < static_cast<float>(q)) k++;
            float dq = static_cast<float>(q - v[k]);
            d[q] = dq * dq + f[v[k]];
        }
    }

    Texture2D BuildCoastSdf(Vector2& originOut, float& worldSizeOut)
    {
        float minX = 1e9f, maxX = -1e9f, minZ = 1e9f, maxZ = -1e9f;
        for (int row = 0; row < data::FieldRows; row++)
        {
            for (int col = 0; col < data::FieldCols; col++)
            {
                data::Vec2 logic = data::CellToLogic(col, row);
                float wx = logic.x * data::RenderScale;
                float wz = logic.y * data::RenderScale;
                minX = fminf(minX, wx);
                maxX = fmaxf(maxX, wx);
                minZ = fminf(minZ, wz);
                maxZ = fmaxf(maxZ, wz);
            }
        }

        float centerX = (minX + maxX) * 0.5f;
        float centerZ = (minZ + maxZ) * 0.5f;
        float half = fmaxf(maxX - minX, maxZ - minZ) * 0.5f + HexCircumradius + SdfMaxDist;
        Vector2 origin = {centerX - half, centerZ - half};
        float worldSize = half * 2.0f;
        int res = SdfResolution;
        float worldPerTexel = worldSize / static_cast<float>(res);

        const float inf = 1e18f;
        std::vector<float> grid(static_cast<std::size_t>(res) * res);
        for (int j = 0; j < res; j++)
        {
            for (int i = 0; i < res; i++)
            {
                float wx = origin.x + (static_cast<float>(i) + 0.5f) * worldPerTexel;
                float wz = origin.y + (static_cast<float>(j) + 0.5f) * worldPerTexel;
                float q = 0.5773503f * wx - 0.3333333f * wz;
                float r = 0.6666667f * wz;
                int col = 0, row = 0;
                RoundToCell(q, r, col, row);
                bool land = col >= 0 && col < data::FieldCols && row >= 0 && row < data::FieldRows;
                grid[static_cast<std::size_t>(j) * res + i] = land ? 0.0f : inf;
            }
        }

        std::vector<float> f(res), d(res), z(res + 1);
        std::vector<int> v(res);
        for (int i = 0; i < res; i++)
        {
            for (int j = 0; j < res; j++) f[j] = grid[static_cast<std::size_t>(j) * res + i];
            SquaredDistance1D(f, d, v, z, res);
            for (int j = 0; j < res; j++) grid[static_cast<std::size_t>(j) * res + i] = d[j];
        }
        for (int j = 0; j < res; j++)
        {
            for (int i = 0; i < res; i++) f[i] = grid[static_cast<std::size_t>(j) * res + i];
            SquaredDistance1D(f, d, v, z, res);
            for (int i = 0; i < res; i++) grid[static_cast<std::size_t>(j) * res + i] = d[i];
        }

        std::vector<unsigned char> pixels(static_cast<std::size_t>(res) * res);
        float invMax = 1.0f / SdfMaxDist;
        for (std::size_t k = 0; k < pixels.size(); k++)
        {
            float dist = std::sqrt(grid[k]) * worldPerTexel;
            float normalized = fminf(fmaxf(dist * invMax, 0.0f), 1.0f);
            pixels[k] = static_cast<unsigned char>(normalized * 255.0f + 0.5f);
        }

        Image image = {pixels.data(), res, res, 1, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE};
        Texture2D texture = LoadTextureFromImage(image);
        SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(texture, TEXTURE_WRAP_CLAMP);

        originOut = origin;
        worldSizeOut = worldSize;
        return texture;
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
    waterShader_ = LoadShader(TextFormat("%swater.vert", dir), TextFormat("%swater.frag", dir));

    Vector3 sunDir = {0.6f, 0.6f, 0.4f};
    float ambient = 0.55f;
    float bands = 3.0f;
    float shadowStrength = 0.4f;
    SetShaderValue(toonShader_, GetShaderLocation(toonShader_, "sunDir"), &sunDir, SHADER_UNIFORM_VEC3);
    SetShaderValue(toonShader_, GetShaderLocation(toonShader_, "ambient"), &ambient, SHADER_UNIFORM_FLOAT);
    SetShaderValue(toonShader_, GetShaderLocation(toonShader_, "bands"), &bands, SHADER_UNIFORM_FLOAT);
    SetShaderValue(toonShader_, GetShaderLocation(toonShader_, "shadowStrength"), &shadowStrength,
                   SHADER_UNIFORM_FLOAT);
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

    normalDepthTarget_ = LoadRenderTexture(width, height);
    colorTarget_ = LoadRenderTexture(width, height);
    shadowTarget_ = LoadShadowmap(ShadowMapSize, ShadowMapSize);
    sdfTexture_ = BuildCoastSdf(sdfOrigin_, sdfWorldSize_);

    waterTimeLoc_ = GetShaderLocation(waterShader_, "time");
    waterSdfMapLoc_ = GetShaderLocation(waterShader_, "sdfMap");
    float sdfMaxDist = SdfMaxDist;
    SetShaderValue(waterShader_, GetShaderLocation(waterShader_, "sdfOrigin"), &sdfOrigin_, SHADER_UNIFORM_VEC2);
    SetShaderValue(waterShader_, GetShaderLocation(waterShader_, "sdfWorldSize"), &sdfWorldSize_,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(waterShader_, GetShaderLocation(waterShader_, "sdfMaxDist"), &sdfMaxDist, SHADER_UNIFORM_FLOAT);
    models_.SetWaterShader(waterShader_);

    Vector3 center = SceneCenterWorld();
    lightCamera_.position = Vector3Add(center, Vector3Scale(Vector3Normalize(sunDir), 60.0f));
    lightCamera_.target = center;
    lightCamera_.up = Vector3{0.0f, 0.0f, 1.0f};
    lightCamera_.fovy = ShadowOrthoSize;
    lightCamera_.projection = CAMERA_ORTHOGRAPHIC;
}

void Renderer::Shutdown()
{
    UnloadRenderTexture(shadowTarget_);
    UnloadRenderTexture(colorTarget_);
    UnloadRenderTexture(normalDepthTarget_);
    UnloadTexture(sdfTexture_);
    UnloadShader(waterShader_);
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

    float elapsed = static_cast<float>(GetTime());
    SetShaderValue(waterShader_, waterTimeLoc_, &elapsed, SHADER_UNIFORM_FLOAT);
    SetWaterUniforms(waterShader_, waterParams_);

    Matrix lightView = MatrixIdentity();
    Matrix lightProj = MatrixIdentity();

    models_.ApplyShader(shadowShader_);
    BeginTextureMode(shadowTarget_);
    ClearBackground(WHITE);
    BeginMode3D(lightCamera_);
    lightView = rlGetMatrixModelview();
    lightProj = rlGetMatrixProjection();
    DrawLand(models_, map);
    EndMode3D();
    EndTextureMode();

    Matrix lightViewProj = MatrixMultiply(lightView, lightProj);
    SetShaderValueMatrix(toonShader_, lightViewProjLoc_, lightViewProj);

    models_.ApplyShader(geomShader_);
    BeginTextureMode(normalDepthTarget_);
    ClearBackground(BLANK);
    rlDisableColorBlend();
    BeginMode3D(camera);
    DrawObjects(models_, map);
    EndMode3D();
    rlEnableColorBlend();
    EndTextureMode();

    models_.ApplyShader(toonShader_);
    BeginTextureMode(colorTarget_);
    ClearBackground(Color{26, 26, 28, 255});
    int shadowSlot = ShadowSlot;
    int sdfSlot = SdfSlot;
    BeginMode3D(camera);
    rlEnableShader(toonShader_.id);
    rlActiveTextureSlot(shadowSlot);
    rlEnableTexture(shadowTarget_.depth.id);
    rlSetUniform(shadowMapLoc_, &shadowSlot, SHADER_UNIFORM_INT, 1);
    DrawLand(models_, map);

    rlEnableShader(waterShader_.id);
    rlActiveTextureSlot(sdfSlot);
    rlEnableTexture(sdfTexture_.id);
    rlSetUniform(waterSdfMapLoc_, &sdfSlot, SHADER_UNIFORM_INT, 1);
    DrawWater(models_);
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
    DrawFPS(10, 10);
    if (overlay) overlay();
    EndDrawing();
}
}
