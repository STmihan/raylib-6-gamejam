#ifndef AI_POLICY_H
#define AI_POLICY_H

#include "data/unit/unit_types.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"

namespace ai
{
enum class ActionKind
{
    None,
    Play,
    Merge,
};

struct Action
{
    ActionKind kind = ActionKind::None;
    int slot = -1;
    int col = 0;
    int row = 0;
    int host = -1;
    int donor = -1;
};

Action DecideAction(const logic::GameState& state, const logic::Map& map, data::Team team);
int ApplyAction(logic::GameState& state, const logic::Map& map, data::Team team, const Action& action);
void RunMicro(logic::GameState& state, const logic::Map& map, data::Team team);
}

#endif
