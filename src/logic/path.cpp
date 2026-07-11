#include "logic/path.h"

#include <array>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

#include "data/space/hex.h"
#include "data/tile/tile.h"

namespace logic
{
bool Passable(const Map& map, const int* occupant, int selfIndex, int col, int row)
{
    if (!map.InBounds(col, row)) return false;
    data::TileType tile = map.At(col, row);
    if (tile != data::TileType::Wall && !data::TileConfigOf(tile).passable) return false;
    int occ = occupant[static_cast<std::size_t>(row) * MapCols + col];
    return occ < 0 || occ == selfIndex;
}

bool FindStep(const Map& map, const int* occupant, int selfIndex, int fromCol, int fromRow, int goalCol,
              int goalRow, int stopRange, int& outCol, int& outRow)
{
    data::Offset goal{goalCol, goalRow};
    if (data::HexDistance({fromCol, fromRow}, goal) <= stopRange) return false;

    std::array<int, MapTileCount> gScore;
    std::array<int, MapTileCount> parent;
    gScore.fill(-1);
    parent.fill(-1);

    using Node = std::pair<int, int>;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;

    int start = fromRow * MapCols + fromCol;
    gScore[static_cast<std::size_t>(start)] = 0;
    open.push({data::HexDistance({fromCol, fromRow}, goal), start});

    int reached = -1;
    while (!open.empty())
    {
        int f = open.top().first;
        int cur = open.top().second;
        open.pop();
        int cc = cur % MapCols;
        int cr = cur / MapCols;
        int g = gScore[static_cast<std::size_t>(cur)];
        if (g < 0) continue;
        if (f != g + data::HexDistance({cc, cr}, goal)) continue;
        if (data::HexDistance({cc, cr}, goal) <= stopRange)
        {
            reached = cur;
            break;
        }
        for (int dir = 0; dir < 6; dir++)
        {
            data::Offset nb = data::Neighbor({cc, cr}, dir);
            if (!Passable(map, occupant, selfIndex, nb.col, nb.row)) continue;
            int ni = nb.row * MapCols + nb.col;
            int ng = g + 1;
            if (gScore[static_cast<std::size_t>(ni)] < 0 || ng < gScore[static_cast<std::size_t>(ni)])
            {
                gScore[static_cast<std::size_t>(ni)] = ng;
                parent[static_cast<std::size_t>(ni)] = cur;
                open.push({ng + data::HexDistance(nb, goal), ni});
            }
        }
    }

    if (reached < 0) return false;
    int node = reached;
    int prev = parent[static_cast<std::size_t>(node)];
    if (prev < 0) return false;
    while (prev != start)
    {
        node = prev;
        prev = parent[static_cast<std::size_t>(node)];
        if (prev < 0) return false;
    }
    outCol = node % MapCols;
    outRow = node / MapCols;
    return true;
}
}
