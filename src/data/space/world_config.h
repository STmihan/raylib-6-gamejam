#ifndef DATA_WORLD_CONFIG_H
#define DATA_WORLD_CONFIG_H

namespace data
{
inline constexpr float HexRadius = 1.0f;
inline constexpr float NeighborDistanceModel = 1.7320508f;
inline constexpr float LogicUnitsPerTile = 100.0f;
inline constexpr float RenderScale = NeighborDistanceModel / LogicUnitsPerTile;

inline constexpr float ColSpacingLogic = 86.60254f;
inline constexpr float RowSpacingLogic = 100.0f;
inline constexpr float OddColumnOffsetLogic = 50.0f;

// Legend:
// R=road
// B=base
// W=wall
// .=field
// T=forest
// D=corner (decorative, behaves as field)
// O=empty (impassable hole, shows water; planes fly over)
inline constexpr const char* FieldLayout = R"(
RRRRRRRRRRRRRRRRRRRRRRRR
RRRRRRRRRRRRRRRRRRRRRRRR
RRRRRRRRRRBBBBRRRRRRRRRR
RRRRRRRRRRBBBBRRRRRRRRRR
RRRRRRRRRRBBBBRRRRRRRRRR
RRRRRRRRRRBBBBRRRRRRRRRR
RRRRRRRRRRRRRRRRRRRRRRRR
WWWWWWWWWWWWWWWWWWWWWWWW
........................
TTTTTTTTTTTTTTTTTTTTTTTT
TTTTTTTTTTTTTTTTTTTTTTTT
TTTTTTTTTTTTTTTTTTTTTTTT
........................
........................
........................
........................
........................
..OOOOOOOOO..OOOOOOOOO..
..OOOOOOOOO..OOOOOOOOO..
..OOOOOOOOO..OOOOOOOOO..
..OOOOOOOOO..OOOOOOOOO..
........................
........................
........................
........................
........................
TTTTTTTTTTTTTTTTTTTTTTTT
TTTTTTTTTTTTTTTTTTTTTTTT
TTTTTTTTTTTTTTTTTTTTTTTT
........................
WWWWWWWWWWWWWWWWWWWWWWWW
RRRRRRRRRRRRRRRRRRRRRRRR
RRRRRRRRRRBBBBRRRRRRRRRR
RRRRRRRRRRBBBBRRRRRRRRRR
RRRRRRRRRRBBBBRRRRRRRRRR
RRRRRRRRRRBBBBRRRRRRRRRR
RRRRRRRRRRRRRRRRRRRRRRRR
RRRRRRRRRRRRRRRRRRRRRRRR
)";

constexpr int CountLayoutRows(const char* layout)
{
    int rows = 0;
    int col = 0;
    for (const char* symbol = layout; *symbol != '\0'; ++symbol)
    {
        if (*symbol == '\n')
        {
            if (col > 0) rows++;
            col = 0;
        }
        else
        {
            col++;
        }
    }
    if (col > 0) rows++;
    return rows;
}

constexpr int CountLayoutCols(const char* layout)
{
    int maxCol = 0;
    int col = 0;
    for (const char* symbol = layout; *symbol != '\0'; ++symbol)
    {
        if (*symbol == '\n')
        {
            if (col > maxCol) maxCol = col;
            col = 0;
        }
        else
        {
            col++;
        }
    }
    if (col > maxCol) maxCol = col;
    return maxCol;
}

inline constexpr int FieldCols = CountLayoutCols(FieldLayout);
inline constexpr int FieldRows = CountLayoutRows(FieldLayout);
}

#endif
