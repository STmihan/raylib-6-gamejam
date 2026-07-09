#ifndef LOGIC_SIMULATION_H
#define LOGIC_SIMULATION_H

#include "data/unit/unit.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"

namespace logic {

class Simulation {
public:
    void Init(GameState &state, const Map &map);
    void Step(GameState &state, float dt);

private:
    const Map *map_ = nullptr;
    int enemyBaseRow_[2] = {0, 0};
};

}

#endif
