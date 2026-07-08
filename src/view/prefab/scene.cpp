#include "view/prefab/scene.h"

#include <cmath>

#include "raylib.h"

#include "data/scene/scene_config.h"
#include "data/space/hex.h"
#include "data/tile/tile.h"
#include "logic/world/map.h"
#include "view/prefab/registries/model_registry.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    constexpr float RadToDeg = 57.2957795f;

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
}

void Scene::Draw(const logic::Map& map, bool includeFloors) const
{
    if (includeFloors) DrawFloors(map);
    DrawStructures(map);
}

void Scene::DrawFloors(const logic::Map& map) const
{
    const ModelRegistry& models = *models_;
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

void Scene::DrawStructures(const logic::Map& map) const
{
    const ModelRegistry& models = *models_;

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

    for (int row = 0; row < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (map.At(col, row) != data::TileType::Forest) continue;
            Vector3 center = CellWorld(col, row, 0.0f);
            for (int k = 0; k < data::TreesPerForest; k++)
            {
                Vector3 offset = data::TreeOffsets[k];
                Vector3 position = {center.x + offset.x, center.y, center.z + offset.z};
                int treeIndex = (col * data::TreeSpeciesHashCol + row * data::TreeSpeciesHashRow + k)
                    % data::TreeSpeciesCount;
                float yaw = static_cast<float>(((col + row + k) * data::TreeYawHash) % 360);
                DrawModelYaw(models.Tree(treeIndex), position, yaw, WHITE);
            }
        }
    }

    for (int row = 0; row + 1 < logic::MapRows; row++)
    {
        for (int col = 0; col < logic::MapCols; col++)
        {
            bool isTopOfPair = map.At(col, row) == data::TileType::Base
                && map.At(col, row + 1) == data::TileType::Base
                && (row == 0 || map.At(col, row - 1) != data::TileType::Base);
            if (!isTopOfPair) continue;

            Vector3 mid = Midpoint(CellWorld(col, row, 0.0f), CellWorld(col, row + 1, 0.0f));
            float yaw = (row >= logic::MapRows / 2 ? data::BaseFlipDegrees : 0.0f)
                + (col % 2 == 0 ? data::BaseFlipDegrees : 0.0f);
            DrawModelYaw(models.BaseSection(), mid, yaw, WHITE);
        }
    }
}
}
