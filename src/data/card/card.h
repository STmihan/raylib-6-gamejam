#ifndef DATA_CARD_CARD_H
#define DATA_CARD_CARD_H

#include <cstddef>

#include "data/balance/balance.h"
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
    const char* name = "Unit";
    const char* portrait = "";
    switch (type)
    {
    case UnitType::Infantry: name = "Infantry"; portrait = "assets/previews/soldier_infantry.png"; break;
    case UnitType::Rocketeer: name = "Rocketeer"; portrait = "assets/previews/soldier_rocket.png"; break;
    case UnitType::Engineer: name = "Engineer"; portrait = "assets/previews/soldier_engineer.png"; break;
    case UnitType::RL: name = "Rocket Launcher"; portrait = "assets/previews/pvo.png"; break;
    case UnitType::Tank: name = "Tank"; portrait = "assets/previews/tank.png"; break;
    case UnitType::Plane: name = "Plane"; portrait = "assets/previews/plane.png"; break;
    }
    return {type, name, portrait, Rules().cardCost[static_cast<std::size_t>(type)],
            Rules().cardCharges[static_cast<std::size_t>(type)]};
}
}

#endif
