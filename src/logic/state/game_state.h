#ifndef LOGIC_GAME_STATE_H
#define LOGIC_GAME_STATE_H

#include <array>
#include <cstdint>

#include "data/sim/sim_config.h"
#include "data/space/vec.h"

namespace logic {

struct Entity {
    bool active;
    std::uint32_t id;
    data::Vec2 position;
    data::Vec2 velocity;
};

struct GameState {
    std::uint64_t tick;
    int entityCount;
    std::array<Entity, data::MaxEntities> entities;
};

}

#endif
