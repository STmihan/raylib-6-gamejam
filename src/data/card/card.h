#ifndef DATA_CARD_CARD_H
#define DATA_CARD_CARD_H

#include "data/unit/unit.h"

namespace data
{
struct CardDef
{
    UnitType type;
    const char* name;
    const char* portrait;
    int cost;
    int charges;
};

inline CardDef CardDefOf(UnitType type)
{
    switch (type)
    {
    case UnitType::Infantry:
        return {type, "Infantry", "assets/previews/soldier_infantry.png", 1, 3};
    case UnitType::Rocketeer:
        return {type, "Rocketeer", "assets/previews/soldier_rocket.png", 1, 2};
    case UnitType::Engineer:
        return {type, "Engineer", "assets/previews/soldier_engineer.png", 1, 1};
    case UnitType::AA:
        return {type, "AA", "assets/previews/pvo.png", 2, 1};
    case UnitType::Tank:
        return {type, "Tank", "assets/previews/tank.png", 2, 1};
    case UnitType::Plane:
        return {type, "Plane", "assets/previews/plane.png", 3, 1};
    }
    return {type, "Unit", "", 1, 1};
}
}

#endif
