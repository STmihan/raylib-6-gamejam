#ifndef DATA_CARD_DECK_CONFIG_H
#define DATA_CARD_DECK_CONFIG_H

#include <iterator>

#include "data/unit/unit.h"

namespace data
{
struct DeckEntry
{
    UnitType type;
    int count;
};

inline constexpr int HandSize = 3;

inline constexpr DeckEntry DeckList[] = {
    {UnitType::Infantry, 2},
    {UnitType::Rocketeer, 2},
    {UnitType::Engineer, 2},
    {UnitType::AA, 2},
    {UnitType::Tank, 2},
    {UnitType::Plane, 2},
};

inline constexpr int DeckEntryCount = std::size(DeckList);

constexpr int TotalDeckCards()
{
    int total = 0;
    for (int i = 0; i < DeckEntryCount; i++) total += DeckList[i].count;
    return total;
}

inline constexpr int MaxDeckCards = TotalDeckCards();
}

#endif
