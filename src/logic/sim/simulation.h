#ifndef LOGIC_SIMULATION_H
#define LOGIC_SIMULATION_H

#include "logic/state/game_state.h"

namespace logic {

class Simulation {
public:
    void Init(GameState &state);
    void Step(GameState &state, float dt);
};

}

#endif
