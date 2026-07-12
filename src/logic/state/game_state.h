#ifndef LOGIC_GAME_STATE_H
#define LOGIC_GAME_STATE_H

#include <array>
#include <cstdint>

#include "data/card/deck_config.h"
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
    int footMinCol;
    int footMinRow;
    int footMaxCol;
    int footMaxRow;
    int hp;
    int targetSlot;
    int forcedTarget;
    float attackCooldown;
    int burstIndex;
    float deployTimer;
    bool hasMoveOrder;
    data::Vec2 moveTarget;
    int armorHits;
    int armorMax;
    int attackRange;
    int aggroRange;
    bool stationary;
    int pathCol;
    int pathRow;
    float repathTimer;
    float lastHitMult;
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
    int aoeRadius;
    float mult;
};

struct HealPulse {
    bool active;
    data::Vec2 center;
    int radius;
    std::uint64_t startTick;
};

struct Beam {
    bool active;
    int attackerSlot;
    int targetSlot;
    int muzzleIndex;
    std::uint64_t startTick;
};

struct MissMark {
    bool active;
    data::Vec2 position;
    std::uint64_t startTick;
};

struct HandSlot {
    data::UnitType type;
    int donor;
    int chargesLeft;
};

struct Deck {
    std::array<data::UnitType, data::MaxDeckCards> cards;
    int head;
    int count;
};

struct PlayerCards {
    Deck deck;
    std::array<HandSlot, data::HandSize> hand;
};

struct GameState {
    std::uint64_t tick;
    std::uint32_t seed;
    int entityCount;
    int winner;
    std::array<float, 2> resource;
    std::array<int, 2> unitsDeployed;
    std::array<Entity, data::MaxEntities> entities;
    std::array<Projectile, data::MaxProjectiles> projectiles;
    std::array<HealPulse, data::MaxHealPulses> healPulses;
    std::array<Beam, data::MaxBeams> beams;
    std::array<MissMark, data::MaxMisses> misses;
    std::array<PlayerCards, 2> players;
};

}

#endif
