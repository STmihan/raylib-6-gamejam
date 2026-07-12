#ifndef DATA_CARD_DECK_CONFIG_H
#define DATA_CARD_DECK_CONFIG_H

#include "data/unit/unit_types.h"

namespace data
{
struct DeckEntry
{
    UnitType type;
    int count;
};

inline constexpr int HandSize = 3;
inline constexpr int MaxDeckEntries = 16;
inline constexpr int MaxDeckCards = 48;
}

#endif
