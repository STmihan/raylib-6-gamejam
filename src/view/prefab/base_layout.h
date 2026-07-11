#ifndef VIEW_PREFAB_BASE_LAYOUT_H
#define VIEW_PREFAB_BASE_LAYOUT_H

#include "raylib.h"

#include "data/space/hex.h"
#include "data/tile/tile.h"
#include "data/unit/unit.h"
#include "logic/world/map.h"
#include "view/space/world_space.h"

namespace view
{
inline bool BaseSectionAt(const logic::Map& map, int col, int row)
{
    if (map.At(col, row) != data::TileType::Base) return false;
    if (row + 1 >= logic::MapRows || map.At(col, row + 1) != data::TileType::Base) return false;
    int above = 0;
    for (int r = row - 1; r >= 0 && map.At(col, r) == data::TileType::Base; r--) above++;
    return above % 2 == 0;
}

inline data::Team BaseSectionOwner(int row)
{
    return row >= logic::MapRows / 2 ? data::Team::Bottom : data::Team::Top;
}

inline Vector3 BaseSectionMidpoint(int col, int row)
{
    Vector3 a = LogicToWorld(data::CellToLogic(col, row), 0.0f);
    Vector3 b = LogicToWorld(data::CellToLogic(col, row + 1), 0.0f);
    return Vector3{(a.x + b.x) * 0.5f, 0.0f, (a.z + b.z) * 0.5f};
}

inline int BaseSectionCount(const logic::Map& map, data::Team team)
{
    int count = 0;
    for (int row = 0; row + 1 < logic::MapRows; row++)
        for (int col = 0; col < logic::MapCols; col++)
            if (BaseSectionAt(map, col, row) && BaseSectionOwner(row) == team) count++;
    return count;
}

inline Vector3 BaseSectionWorld(const logic::Map& map, data::Team team, int index)
{
    int seen = 0;
    for (int row = 0; row + 1 < logic::MapRows; row++)
        for (int col = 0; col < logic::MapCols; col++)
        {
            if (!BaseSectionAt(map, col, row) || BaseSectionOwner(row) != team) continue;
            if (seen == index) return BaseSectionMidpoint(col, row);
            seen++;
        }
    return Vector3{0.0f, 0.0f, 0.0f};
}
}

#endif
