#include "logic/path.h"

#include <array>
#include <cstdint>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

#include "data/space/hex.h"
#include "data/tile/tile.h"

namespace logic
{
namespace
{
thread_local std::uint64_t g_findStepCalls = 0;

thread_local std::array<bool, MapTileCount> g_staticPass;
thread_local const Map* g_cachedMap = nullptr;
thread_local std::array<int, MapTileCount> g_gScore;
thread_local std::array<int, MapTileCount> g_parent;
thread_local std::array<std::uint32_t, MapTileCount> g_stamp;
thread_local std::uint32_t g_gen = 0;
}

std::uint64_t FindStepCalls() { return g_findStepCalls; }
void ResetFindStepCalls() { g_findStepCalls = 0; }

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

    g_findStepCalls++;

    if (g_cachedMap != &map)
    {
        for (int row = 0; row < MapRows; row++)
        {
            for (int col = 0; col < MapCols; col++)
            {
                data::TileType tile = map.At(col, row);
                bool pass = tile == data::TileType::Wall || data::TileConfigOf(tile).passable;
                g_staticPass[static_cast<std::size_t>(row) * MapCols + col] = pass;
            }
        }
        g_cachedMap = &map;
    }

    g_gen++;
    if (g_gen == 0)
    {
        g_stamp.fill(0);
        g_gen = 1;
    }

    using Node = std::pair<int, int>;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> open;

    int start = fromRow * MapCols + fromCol;
    g_stamp[static_cast<std::size_t>(start)] = g_gen;
    g_gScore[static_cast<std::size_t>(start)] = 0;
    g_parent[static_cast<std::size_t>(start)] = -1;
    open.push({data::HexDistance({fromCol, fromRow}, goal), start});

    int reached = -1;
    while (!open.empty())
    {
        int f = open.top().first;
        int cur = open.top().second;
        open.pop();
        if (g_stamp[static_cast<std::size_t>(cur)] != g_gen) continue;
        int cc = cur % MapCols;
        int cr = cur / MapCols;
        int g = g_gScore[static_cast<std::size_t>(cur)];
        if (f != g + data::HexDistance({cc, cr}, goal)) continue;
        if (data::HexDistance({cc, cr}, goal) <= stopRange)
        {
            reached = cur;
            break;
        }
        for (int dir = 0; dir < 6; dir++)
        {
            data::Offset nb = data::Neighbor({cc, cr}, dir);
            if (!map.InBounds(nb.col, nb.row)) continue;
            int ni = nb.row * MapCols + nb.col;
            if (!g_staticPass[static_cast<std::size_t>(ni)]) continue;
            int occ = occupant[static_cast<std::size_t>(ni)];
            if (occ >= 0 && occ != selfIndex) continue;
            int ng = g + 1;
            bool seen = g_stamp[static_cast<std::size_t>(ni)] == g_gen;
            if (!seen || ng < g_gScore[static_cast<std::size_t>(ni)])
            {
                g_stamp[static_cast<std::size_t>(ni)] = g_gen;
                g_gScore[static_cast<std::size_t>(ni)] = ng;
                g_parent[static_cast<std::size_t>(ni)] = cur;
                open.push({ng + data::HexDistance(nb, goal), ni});
            }
        }
    }

    if (reached < 0) return false;
    int node = reached;
    int prev = g_parent[static_cast<std::size_t>(node)];
    if (prev < 0) return false;
    while (prev != start)
    {
        node = prev;
        prev = g_parent[static_cast<std::size_t>(node)];
        if (prev < 0) return false;
    }
    outCol = node % MapCols;
    outRow = node / MapCols;
    return true;
}
}
