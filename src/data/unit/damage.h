#ifndef DATA_UNIT_DAMAGE_H
#define DATA_UNIT_DAMAGE_H

#include "data/unit/unit.h"

namespace data
{
inline constexpr float DamageMatrix[UnitTypeCount][UnitTypeCount] = {
    {1.00f, 1.00f, 1.00f, 0.30f, 0.30f, 0.00f},
    {0.30f, 0.30f, 0.30f, 1.00f, 1.00f, 0.80f},
    {0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f},
    {0.10f, 0.10f, 0.50f, 0.50f, 0.50f, 1.00f},
    {1.50f, 1.00f, 1.00f, 1.00f, 1.00f, 0.30f},
    {0.50f, 0.50f, 1.00f, 1.20f, 1.20f, 1.00f},
};

inline constexpr float StructureDamageMultiplier = 1.0f;

inline float DamageMultiplier(UnitType attacker, UnitType defender)
{
    return DamageMatrix[static_cast<int>(attacker)][static_cast<int>(defender)];
}
}

#endif
