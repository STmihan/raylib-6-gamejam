#ifndef DATA_HEX_H
#define DATA_HEX_H

#include <cstdlib>

#include "data/vec.h"
#include "data/world_config.h"

namespace data {

struct Offset {
    int col;
    int row;
};

struct Cube {
    int x;
    int y;
    int z;
};

inline Cube OffsetToCube(Offset o) {
    int x = o.col;
    int z = o.row - (o.col - (o.col & 1)) / 2;
    int y = -x - z;
    return { x, y, z };
}

inline Offset CubeToOffset(Cube c) {
    int col = c.x;
    int row = c.z + (c.x - (c.x & 1)) / 2;
    return { col, row };
}

inline int HexDistance(Offset a, Offset b) {
    Cube ca = OffsetToCube(a);
    Cube cb = OffsetToCube(b);
    return (std::abs(ca.x - cb.x) + std::abs(ca.y - cb.y) + std::abs(ca.z - cb.z)) / 2;
}

inline Offset Neighbor(Offset o, int direction) {
    static const Cube directions[6] = {
        { +1, -1, 0 }, { +1, 0, -1 }, { 0, +1, -1 },
        { -1, +1, 0 }, { -1, 0, +1 }, { 0, -1, +1 },
    };
    Cube c = OffsetToCube(o);
    Cube d = directions[direction];
    return CubeToOffset({ c.x + d.x, c.y + d.y, c.z + d.z });
}

inline Vec2 CellToLogic(int col, int row) {
    float x = ColSpacingLogic * static_cast<float>(col);
    float y = RowSpacingLogic * static_cast<float>(row) + OddColumnOffsetLogic * static_cast<float>(col & 1);
    return { x, y };
}

inline Vec2 CellToLogic(Offset o) {
    return CellToLogic(o.col, o.row);
}

inline Vec2 FieldCenterLogic() {
    return {
        ColSpacingLogic * static_cast<float>(FieldCols - 1) * 0.5f,
        RowSpacingLogic * static_cast<float>(FieldRows - 1) * 0.5f,
    };
}

inline Vec2 FieldExtentLogic() {
    return {
        ColSpacingLogic * static_cast<float>(FieldCols - 1),
        RowSpacingLogic * static_cast<float>(FieldRows - 1) + OddColumnOffsetLogic,
    };
}

}

#endif
