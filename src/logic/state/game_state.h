#ifndef LOGIC_GAME_STATE_H
#define LOGIC_GAME_STATE_H

#include <array>
#include <cstdint>

#include "data/sim/sim_config.h"
#include "data/space/vec.h"
#include "data/unit/unit.h"

namespace logic {

enum class EntityKind {
    Unit,
    Base,
    Wall,
};

struct Entity {
    bool active;
    std::uint32_t id;
    EntityKind kind;
    data::UnitType type;
    data::Team team;
    data::Vec2 position;
    int col;
    int row;
    int hp;
    int targetSlot;
    float attackCooldown;
    int burstIndex;
};

struct Projectile {
    bool active;
    bool arc;
    data::Team team;
    int attackerSlot;
    int muzzleIndex;
    int targetSlot;
    data::Vec2 origin;
    data::Vec2 target;
    std::uint64_t launchTick;
    std::uint64_t impactTick;
    int damage;
};

struct GameState {
    std::uint64_t tick;
    int entityCount;
    int winner;
    std::array<Entity, data::MaxEntities> entities;
    std::array<Projectile, data::MaxProjectiles> projectiles;
};

}

#endif
