#ifndef DATA_UNIT_UNIT_H
#define DATA_UNIT_UNIT_H

#include "data/balance/balance.h"
#include "data/unit/unit_types.h"

namespace data
{
inline UnitStats UnitStatsOf(UnitType type)
{
    return Rules().units[static_cast<std::size_t>(type)];
}

inline UnitStats MergedStats(UnitType type, int donor)
{
    UnitStats s = UnitStatsOf(type);
    if (donor < 0) return s;
    if (donor == static_cast<int>(UnitType::Tank))
    {
        if (s.armorHits < 2) s.armorHits = 2;
    }
    else if (donor == static_cast<int>(UnitType::Rocketeer))
    {
        if (type != UnitType::Plane)
        {
            s.attackRange += 2;
            s.aggroRange += 2;
        }
    }
    else if (donor == static_cast<int>(UnitType::RL))
    {
        s.stationary = true;
        s.attackRange += 1;
        s.aggroRange += 1;
    }
    if (s.aggroRange < s.attackRange) s.aggroRange = s.attackRange;
    return s;
}

inline int MuzzleCount(UnitType type)
{
    switch (type)
    {
    case UnitType::RL: return 4;
    case UnitType::Plane: return 2;
    case UnitType::Engineer: return 0;
    default: return 1;
    }
}

inline float EffectiveMoveModifier(UnitType type, float tileModifier)
{
    UnitStats stats = UnitStatsOf(type);
    if (stats.ignoresAllTileMods) return 1.0f;
    if (stats.ignoresSlow && tileModifier < 1.0f) return 1.0f;
    return tileModifier;
}
}

#endif
