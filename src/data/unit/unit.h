#ifndef DATA_UNIT_UNIT_H
#define DATA_UNIT_UNIT_H

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
};

inline UnitStats UnitStatsOf(UnitType type)
{
    switch (type)
    {
    case UnitType::Infantry: return {60, 55.0f, true, false, false, false, 15.0f, 2, 1.0f, 15, false, 0};
    case UnitType::Rocketeer: return {50, 50.0f, true, false, false, true, 15.0f, 4, 1.5f, 15, false, 0};
    case UnitType::Engineer: return {70, 50.0f, true, false, false, false, 15.0f, 1, 1.0f, 0, false, 0};
    case UnitType::AA: return {140, 40.0f, false, false, true, true, 55.0f, 3, 0.8f, 20, true, 0};
    case UnitType::Tank: return {260, 45.0f, false, false, true, true, 60.0f, 2, 1.3f, 40, false, 3};
    case UnitType::Plane: return {120, 90.0f, false, true, true, true, 0.0f, 1, 1.0f, 30, false, 0};
    }
    return {60, 50.0f, false, false, false, false, 15.0f, 2, 1.0f, 10, false, 0};
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

inline float EffectiveMoveModifier(UnitType type, float tileModifier)
{
    UnitStats stats = UnitStatsOf(type);
    if (stats.ignoresAllTileMods) return 1.0f;
    if (stats.ignoresSlow && tileModifier < 1.0f) return 1.0f;
    return tileModifier;
}
}

#endif
