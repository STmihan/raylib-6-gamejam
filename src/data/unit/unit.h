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

inline int MuzzleCount(UnitType type)
{
    switch (type)
    {
    case UnitType::AA: return 4;
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
