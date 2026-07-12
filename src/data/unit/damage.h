#ifndef DATA_UNIT_DAMAGE_H
#define DATA_UNIT_DAMAGE_H

#include <cstddef>

#include "data/balance/balance.h"
#include "data/unit/unit_types.h"

namespace data
{
inline float DamageMultiplier(UnitType attacker, UnitType defender)
{
    return Rules().damageMatrix[static_cast<std::size_t>(attacker)][static_cast<std::size_t>(defender)];
}

inline float StructureDamageMultiplier()
{
    return Rules().structureDamageMultiplier;
}
}

#endif
