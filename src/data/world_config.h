#ifndef DATA_WORLD_CONFIG_H
#define DATA_WORLD_CONFIG_H

namespace data {

inline constexpr float HexRadius = 1.0f;
inline constexpr float NeighborDistanceModel = 1.7320508f;
inline constexpr float LogicUnitsPerTile = 100.0f;
inline constexpr float RenderScale = NeighborDistanceModel / LogicUnitsPerTile;

inline constexpr int FieldCols = 12;
inline constexpr int FieldRows = 17;

// Legend:
// R=road
// B=base
// W=wall
// .=field
// T=forest
// E=swamp edge
// C=swamp center
// X=red border
inline constexpr const char *FieldLayout = R"(
RRRRRRRRRRRR
RRRRRBBRRRRR
RRRRRBBRRRRR
RRRRRRRRRRRR
WWWWWWWWWWWW
............
............
TTTTTTTTTTTT
EEEEEEEEEEEE
ECCCCEECCCCE
EEEEEEEEEEEE
TTTTTTTTTTTT
............
............
WWWWWWWWWWWW
RRRRRRRRRRRR
RRRRRBBRRRRR
RRRRRBBRRRRR
RRRRRRRRRRRR
)";

inline constexpr float ColSpacingLogic = 86.60254f;
inline constexpr float RowSpacingLogic = 100.0f;
inline constexpr float OddColumnOffsetLogic = 50.0f;

}

#endif
