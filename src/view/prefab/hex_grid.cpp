#include "view/prefab/hex_grid.h"

#include <cmath>
#include <vector>

#include "raymath.h"
#include "rlgl.h"

#include "data/tile/tile.h"
#include "logic/world/map.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    constexpr float OuterRadius = 0.98f;
    constexpr float GridY = 0.03f;

    void Corner(Vector3 center, float radius, int k, float* out)
    {
        float angle = (60.0f * static_cast<float>(k) - 30.0f) * 0.01745329f;
        out[0] = center.x + radius * std::cos(angle);
        out[1] = GridY;
        out[2] = center.z + radius * std::sin(angle);
    }
}

void HexGrid::Load(const logic::Map& map)
{
    map_ = &map;
    BuildMesh();
    material_ = LoadMaterialDefault();
    builtThickness_ = thickness_;
    loaded_ = true;
}

void HexGrid::BuildMesh()
{
    float innerRadius = OuterRadius - thickness_;

    std::vector<int> landCells;
    for (int row = 0; row < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            data::TileType tile = map_->At(col, row);
            if (tile == data::TileType::RedBorder || tile == data::TileType::Empty) continue;
            landCells.push_back(row * logic::MapCols + col);
        }
    }

    int vertexCount = static_cast<int>(landCells.size()) * 6 * 6;
    std::vector<float> vertices(static_cast<std::size_t>(vertexCount) * 3);
    std::vector<float> normals(static_cast<std::size_t>(vertexCount) * 3);
    std::vector<unsigned char> colors(static_cast<std::size_t>(vertexCount) * 4);

    int v = 0;
    auto push = [&](const float* p)
    {
        vertices[static_cast<std::size_t>(v) * 3 + 0] = p[0];
        vertices[static_cast<std::size_t>(v) * 3 + 1] = p[1];
        vertices[static_cast<std::size_t>(v) * 3 + 2] = p[2];
        normals[static_cast<std::size_t>(v) * 3 + 1] = 1.0f;
        colors[static_cast<std::size_t>(v) * 4 + 0] = 255;
        colors[static_cast<std::size_t>(v) * 4 + 1] = 255;
        colors[static_cast<std::size_t>(v) * 4 + 2] = 255;
        colors[static_cast<std::size_t>(v) * 4 + 3] = 255;
        v++;
    };

    for (int cell : landCells)
    {
        int col = cell % logic::MapCols;
        int row = cell / logic::MapCols;
        Vector3 center = CellWorld(col, row, 0.0f);

        for (int k = 0; k < 6; k++)
        {
            float oa[3], ob[3], ia[3], ib[3];
            Corner(center, OuterRadius, k, oa);
            Corner(center, OuterRadius, (k + 1) % 6, ob);
            Corner(center, innerRadius, k, ia);
            Corner(center, innerRadius, (k + 1) % 6, ib);
            push(oa); push(ob); push(ib);
            push(oa); push(ib); push(ia);
        }
    }

    mesh_ = Mesh{};
    mesh_.vertexCount = vertexCount;
    mesh_.triangleCount = vertexCount / 3;
    mesh_.vertices = static_cast<float*>(MemAlloc(static_cast<unsigned int>(vertexCount) * 3 * sizeof(float)));
    mesh_.normals = static_cast<float*>(MemAlloc(static_cast<unsigned int>(vertexCount) * 3 * sizeof(float)));
    mesh_.colors = static_cast<unsigned char*>(MemAlloc(static_cast<unsigned int>(vertexCount) * 4));
    for (std::size_t i = 0; i < vertices.size(); i++) mesh_.vertices[i] = vertices[i];
    for (std::size_t i = 0; i < normals.size(); i++) mesh_.normals[i] = normals[i];
    for (std::size_t i = 0; i < colors.size(); i++) mesh_.colors[i] = colors[i];
    UploadMesh(&mesh_, false);
}

void HexGrid::Unload()
{
    if (!loaded_) return;
    UnloadMesh(mesh_);
    UnloadMaterial(material_);
    loaded_ = false;
}

void HexGrid::Draw()
{
    if (!loaded_) return;

    if (thickness_ != builtThickness_)
    {
        UnloadMesh(mesh_);
        BuildMesh();
        builtThickness_ = thickness_;
    }

    material_.maps[MATERIAL_MAP_DIFFUSE].color = Color{
        static_cast<unsigned char>(color_.x * 255.0f),
        static_cast<unsigned char>(color_.y * 255.0f),
        static_cast<unsigned char>(color_.z * 255.0f),
        static_cast<unsigned char>(opacity_ * 255.0f)};
    rlDisableBackfaceCulling();
    DrawMesh(mesh_, material_, MatrixIdentity());
    rlEnableBackfaceCulling();
}
}
