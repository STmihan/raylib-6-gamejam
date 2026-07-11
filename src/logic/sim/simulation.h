#ifndef LOGIC_SIMULATION_H
#define LOGIC_SIMULATION_H

#include <array>

#include "data/unit/unit.h"
#include "logic/state/game_state.h"
#include "logic/world/map.h"

namespace logic {

class Simulation {
public:
    void Init(GameState &state, const Map &map);
    void Step(GameState &state, float dt);
    static int Deploy(GameState &state, data::UnitType type, int donor, data::Team team, int col, int row);
    static void CommandTarget(GameState &state, int slot, int targetSlot);
    static void CommandMove(GameState &state, int slot, int col, int row);

private:
    const Map *map_ = nullptr;
    int enemyBaseCol_[2] = {0, 0};
    int enemyBaseRow_[2] = {0, 0};
    data::Vec2 enemyBasePos_[2] = {};
    std::array<int, MapTileCount> occupant_{};
};

}

#endif
