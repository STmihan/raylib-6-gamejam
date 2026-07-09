#ifndef DATA_HEX_H
#define DATA_HEX_H

#include <cmath>
#include <cstdlib>

#include "data/space/vec.h"
#include "data/space/world_config.h"

namespace data
{
struct Offset
{
    int col;
    int row;
};

struct Cube
{
    int x;
    int y;
    int z;
};

inline Cube OffsetToCube(Offset o)
{
    int x = o.col - (o.row - (o.row & 1)) / 2;
    int z = o.row;
    return {x, -x - z, z};
}

inline Offset CubeToOffset(Cube c)
{
    return {c.x + (c.z - (c.z & 1)) / 2, c.z};
}

inline int HexDistance(Offset a, Offset b)
{
    Cube ca = OffsetToCube(a);
    Cube cb = OffsetToCube(b);
    return (std::abs(ca.x - cb.x) + std::abs(ca.y - cb.y) + std::abs(ca.z - cb.z)) / 2;
}

inline Offset Neighbor(Offset o, int direction)
{
    static const Cube directions[6] = {
        {+1, -1, 0}, {+1, 0, -1}, {0, +1, -1},
        {-1, +1, 0}, {-1, 0, +1}, {0, -1, +1},
    };
    Cube c = OffsetToCube(o);
    Cube d = directions[direction];
    return CubeToOffset({c.x + d.x, c.y + d.y, c.z + d.z});
}

inline Vec2 CellToLogic(int col, int row)
{
    float x = RowSpacingLogic * static_cast<float>(col) + OddColumnOffsetLogic * static_cast<float>(row & 1);
    float y = ColSpacingLogic * static_cast<float>(row);
    return {x, y};
}

inline Vec2 CellToLogic(Offset o)
{
    return CellToLogic(o.col, o.row);
}

inline Offset CellFromLogic(Vec2 p)
{
    int row = static_cast<int>(std::floor(p.y / ColSpacingLogic + 0.5f));
    int col = static_cast<int>(std::floor((p.x - OddColumnOffsetLogic * static_cast<float>(row & 1))
        / RowSpacingLogic + 0.5f));
    return {col, row};
}

inline Vec2 FieldCenterLogic()
{
    Vec2 a = CellToLogic(0, 0);
    Vec2 b = CellToLogic(FieldCols - 1, FieldRows - 1);
    return {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
}

inline Vec2 FieldExtentLogic()
{
    return CellToLogic(FieldCols - 1, FieldRows - 1);
}
}

#endif
