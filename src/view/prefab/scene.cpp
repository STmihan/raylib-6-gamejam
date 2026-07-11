#include "view/prefab/scene.h"

#include <cmath>

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "data/scene/scene_config.h"
#include "data/space/hex.h"
#include "data/tile/tile.h"
#include "logic/world/map.h"
#include "view/prefab/base_layout.h"
#include "view/prefab/registries/model_registry.h"
#include "view/space/orientation.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{

    void DrawTrees(const ModelRegistry& models, int col, int row, Color tint)
    {
        Vector3 center = CellWorld(col, row, 0.0f);
        for (int k = 0; k < data::TreesPerForest; k++)
        {
            Vector3 offset = data::TreeOffsets[k];
            Vector3 position = {center.x + offset.x, center.y, center.z + offset.z};
            int treeIndex = (col * data::TreeSpeciesHashCol + row * data::TreeSpeciesHashRow + k)
                % data::TreeSpeciesCount;
            float yaw = static_cast<float>(((col + row + k) * data::TreeYawHash) % 360);
            DrawModelYaw(models.Tree(treeIndex), position, yaw, tint);
        }
    }

    bool Occluded(const bool* occluded, int col, int row)
    {
        return occluded != nullptr && occluded[static_cast<std::size_t>(row) * logic::MapCols + col];
    }
}

void Scene::Draw(const logic::Map& map, bool includeFloors, const bool* occluded, const bool* wallAlive) const
{
    if (includeFloors) DrawFloors(map);
    DrawStructures(map, occluded, wallAlive);
}

void Scene::DrawFloors(const logic::Map& map) const
{
    const ModelRegistry& models = *models_;
    for (int row = 0; row < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            data::TileType type = map.At(col, row);
            if (type == data::TileType::Empty) continue;
            DrawModelYaw(models.FloorFor(type), CellWorld(col, row, 0.0f), 0.0f, WHITE);
        }
    }
}

void Scene::DrawStructures(const logic::Map& map, const bool* occluded, const bool* wallAlive) const
{
    const ModelRegistry& models = *models_;

    for (int row = 0; row < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (map.At(col, row) != data::TileType::Wall) continue;
            if (Occluded(occluded, col, row)) continue;
            if (wallAlive != nullptr && !wallAlive[static_cast<std::size_t>(row) * logic::MapCols + col]) continue;
            DrawModelYaw(models.Wall(), CellWorld(col, row, 0.0f), 0.0f, WHITE);
        }
    }

    for (int row = 0; row < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (map.At(col, row) != data::TileType::Forest) continue;
            if (Occluded(occluded, col, row)) continue;
            DrawTrees(models, col, row, WHITE);
        }
    }

    for (int row = 0; row + 1 < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (!BaseSectionAt(map, col, row)) continue;

            Vector3 mid = Midpoint(CellWorld(col, row, 0.0f), CellWorld(col, row + 1, 0.0f));
            float yaw = (row >= logic::MapRows / 2 ? data::BaseFlipDegrees : 0.0f)
                + (col % 2 == 0 ? data::BaseFlipDegrees : 0.0f);
            DrawModelYaw(models.BaseSection(), mid, yaw, WHITE);

            int idx = data::TeamIndex(row >= logic::MapRows / 2 ? data::Team::Bottom : data::Team::Top);
            data::Team owner = row >= logic::MapRows / 2 ? data::Team::Bottom : data::Team::Top;
            float turretYaw = baseHasTarget_[idx]
                ? YawTowards(Vector2{mid.x, mid.z}, Vector2{baseAim_[idx].x, baseAim_[idx].z}, TeamYaw(owner))
                : TeamYaw(owner);
            DrawModelYaw(models.BaseTurret(), mid, turretYaw, WHITE);
        }
    }
}

void Scene::SetBaseAim(data::Team team, Vector3 aim, bool hasTarget)
{
    int idx = data::TeamIndex(team);
    baseAim_[idx] = aim;
    baseHasTarget_[idx] = hasTarget;
}

void Scene::DrawBaseHighlight(const logic::Map& map, data::Team team, Color tint) const
{
    const ModelRegistry& models = *models_;
    for (int row = 0; row + 1 < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (!BaseSectionAt(map, col, row)) continue;

            data::Team owner = row >= logic::MapRows / 2 ? data::Team::Bottom : data::Team::Top;
            if (owner != team) continue;

            Vector3 mid = Midpoint(CellWorld(col, row, 0.0f), CellWorld(col, row + 1, 0.0f));
            float yaw = (row >= logic::MapRows / 2 ? data::BaseFlipDegrees : 0.0f)
                + (col % 2 == 0 ? data::BaseFlipDegrees : 0.0f);
            DrawModelYaw(models.BaseSection(), mid, yaw, tint);
        }
    }
}

void Scene::DrawGhostStructures(const logic::Map& map, const bool* occluded, const bool* wallAlive,
                                unsigned char alpha) const
{
    if (occluded == nullptr) return;
    const ModelRegistry& models = *models_;
    Color tint = {255, 255, 255, alpha};

    BeginBlendMode(BLEND_ALPHA);
    rlDisableDepthMask();
    for (int row = 0; row < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (!Occluded(occluded, col, row)) continue;
            data::TileType type = map.At(col, row);
            if (type == data::TileType::Wall)
            {
                if (wallAlive != nullptr && !wallAlive[static_cast<std::size_t>(row) * logic::MapCols + col])
                    continue;
                DrawModelYaw(models.Wall(), CellWorld(col, row, 0.0f), 0.0f, tint);
            }
            else if (type == data::TileType::Forest)
            {
                DrawTrees(models, col, row, tint);
            }
        }
    }
    rlEnableDepthMask();
    EndBlendMode();
}
}
