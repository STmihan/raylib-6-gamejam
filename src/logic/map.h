#ifndef LOGIC_MAP_H
#define LOGIC_MAP_H

#include <array>

#include "data/tile.h"
#include "data/world_config.h"

namespace logic {

inline constexpr int MapCols = data::FieldCols;
inline constexpr int MapRows = data::FieldRows;
inline constexpr int MapTileCount = MapCols * MapRows;

struct Map {
    std::array<data::TileType, MapTileCount> tiles;

    bool InBounds(int col, int row) const {
        return col >= 0 && col < MapCols && row >= 0 && row < MapRows;
    }

    data::TileType At(int col, int row) const {
        return tiles[static_cast<std::size_t>(row) * MapCols + col];
    }

    void Set(int col, int row, data::TileType type) {
        tiles[static_cast<std::size_t>(row) * MapCols + col] = type;
    }
};

Map BuildMap();

}

#endif
