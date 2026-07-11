#ifndef LOGIC_CARDS_H
#define LOGIC_CARDS_H

#include <cstdint>

#include "data/unit/unit.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"

namespace logic
{
void InitPlayerCards(GameState& state, std::uint32_t seed);
int SlotCost(const HandSlot& slot);
bool CanMerge(const PlayerCards& player, int host, int donor);
void MergeSlots(GameState& state, data::Team team, int host, int donor);
int PlayCard(GameState& state, const Map& map, data::Team team, int slot, int col, int row);
}

#endif
