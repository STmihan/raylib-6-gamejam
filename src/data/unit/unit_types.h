#ifndef DATA_UNIT_TYPES_H
#define DATA_UNIT_TYPES_H

namespace data
{
enum class UnitType
{
    Infantry,
    Rocketeer,
    Engineer,
    AA,
    Tank,
    Plane,
};

inline constexpr int UnitTypeCount = 6;

enum class Team
{
    Top,
    Bottom,
};

inline constexpr Team PlayerTeam = Team::Bottom;
inline constexpr Team EnemyTeam = Team::Top;

inline int TeamIndex(Team team)
{
    return team == Team::Top ? 0 : 1;
}

struct UnitStats
{
    int hp;
    float moveSpeed;
    bool ignoresSlow;
    bool ignoresAllTileMods;
    bool isVehicle;
    bool canTargetAir;
    float footprint;
    int attackRange;
    float attackInterval;
    int baseDamage;
    bool stationary;
    int armorHits;
    int aoeRadius;
};

inline const char* UnitTypeName(UnitType type)
{
    switch (type)
    {
    case UnitType::Infantry: return "Infantry";
    case UnitType::Rocketeer: return "Rocketeer";
    case UnitType::Engineer: return "Engineer";
    case UnitType::AA: return "AA";
    case UnitType::Tank: return "Tank";
    case UnitType::Plane: return "Plane";
    }
    return "Unit";
}
}

#endif
