#ifndef LOGIC_DEPLOY_H
#define LOGIC_DEPLOY_H

#include "data/sim/sim_config.h"
#include "data/space/hex.h"
#include "data/tile/tile.h"
#include "data/unit/unit.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"

namespace logic
{
inline bool IsDeployable(const Map& map, int col, int row, data::Team team, bool anywhere)
{
    if (!map.InBounds(col, row)) return false;
    bool ownHalf = team == data::Team::Bottom ? row >= MapRows / 2 : row < MapRows / 2;
    if (!ownHalf) return false;
    data::TileType tile = map.At(col, row);
    if (anywhere)
    {
        return tile == data::TileType::ConcreteRoad || tile == data::TileType::Field
            || tile == data::TileType::Forest;
    }
    return tile == data::TileType::ConcreteRoad;
}

inline bool CellHasUnit(const GameState& state, int col, int row)
{
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const Entity& e = state.entities[i];
        if (e.active && e.kind == EntityKind::Unit && e.col == col && e.row == row) return true;
    }
    return false;
}

inline float PickRadius(const Entity& e)
{
    if (e.kind == EntityKind::Base) return 120.0f;
    if (e.kind == EntityKind::Wall) return 55.0f;
    float radius = data::UnitStatsOf(e.type).footprint;
    return radius < 45.0f ? 45.0f : radius;
}

inline int PickFriendlyUnit(const GameState& state, data::Vec2 pos, data::Team team)
{
    int best = -1;
    float bestDistSq = 0.0f;
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const Entity& e = state.entities[i];
        if (!e.active || e.kind != EntityKind::Unit || e.team != team) continue;
        float radius = PickRadius(e);
        float dx = e.position.x - pos.x;
        float dy = e.position.y - pos.y;
        float distSq = dx * dx + dy * dy;
        if (distSq > radius * radius) continue;
        if (best < 0 || distSq < bestDistSq)
        {
            bestDistSq = distSq;
            best = i;
        }
    }
    return best;
}

inline int PickEnemyTarget(const GameState& state, data::Vec2 pos, data::Team team, const Map& map)
{
    data::Offset cell = data::CellFromLogic(pos);
    bool enemyHalf = team == data::Team::Bottom ? cell.row < MapRows / 2 : cell.row >= MapRows / 2;
    if (map.InBounds(cell.col, cell.row) && map.At(cell.col, cell.row) == data::TileType::Base && enemyHalf)
    {
        for (int i = 0; i < data::MaxEntities; i++)
        {
            const Entity& e = state.entities[i];
            if (e.active && e.kind == EntityKind::Base && e.team != team) return i;
        }
    }

    int best = -1;
    float bestDistSq = 0.0f;
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const Entity& e = state.entities[i];
        if (!e.active || e.team == team || e.kind == EntityKind::Base) continue;
        float radius = PickRadius(e);
        float dx = e.position.x - pos.x;
        float dy = e.position.y - pos.y;
        float distSq = dx * dx + dy * dy;
        if (distSq > radius * radius) continue;
        if (best < 0 || distSq < bestDistSq)
        {
            bestDistSq = distSq;
            best = i;
        }
    }
    return best;
}
}

#endif
