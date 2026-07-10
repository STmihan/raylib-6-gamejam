#include "logic/sim/simulation.h"

#include <climits>
#include <cmath>
#include <limits>

#include "data/economy/economy.h"
#include "data/sim/sim_config.h"
#include "data/space/hex.h"
#include "data/tile/tile.h"
#include "data/time/time_config.h"
#include "data/unit/damage.h"

namespace logic {

namespace {

bool CanDamage(const Entity &attacker, const Entity &target) {
    if (target.kind == EntityKind::Unit) {
        bool targetIsAir = target.type == data::UnitType::Plane;
        if (targetIsAir && !data::UnitStatsOf(attacker.type).canTargetAir) return false;
        return data::DamageMultiplier(attacker.type, target.type) > 0.0f;
    }
    return true;
}

bool WallBlocksPath(const Entity &unit, const Entity &wall) {
    if (unit.type == data::UnitType::Plane) return false;
    bool ahead = unit.team == data::Team::Top ? wall.row > unit.row : wall.row < unit.row;
    if (!ahead) return false;
    float dx = wall.position.x - unit.position.x;
    return (dx < 0.0f ? -dx : dx) < data::RowSpacingLogic * 0.51f;
}

int ComputeDamage(const Entity &attacker, const Entity &target) {
    float multiplier = target.kind == EntityKind::Unit
        ? data::DamageMultiplier(attacker.type, target.type)
        : data::StructureDamageMultiplier;
    int damage = static_cast<int>(static_cast<float>(data::UnitStatsOf(attacker.type).baseDamage) * multiplier);
    return damage > 0 ? damage : 0;
}

int AcquireAlly(const GameState &state, int index) {
    const Entity &self = state.entities[index];
    data::Offset here = {self.col, self.row};
    int best = -1;
    int bestDistance = INT_MAX;
    for (int j = 0; j < data::MaxEntities; j++) {
        if (j == index) continue;
        const Entity &other = state.entities[j];
        if (!other.active || other.kind != EntityKind::Unit || other.team != self.team) continue;
        if (!data::UnitStatsOf(other.type).isVehicle) continue;
        int distance = data::HexDistance(here, {other.col, other.row});
        if (distance <= data::UnitStatsOf(self.type).attackRange && distance < bestDistance) {
            bestDistance = distance;
            best = j;
        }
    }
    return best;
}

bool UsesProjectile(data::UnitType type) {
    return type == data::UnitType::AA || type == data::UnitType::Rocketeer || type == data::UnitType::Tank;
}

void FireProjectile(GameState &state, int attackerIndex, int targetSlot, int muzzleIndex) {
    const Entity &attacker = state.entities[attackerIndex];
    const Entity &target = state.entities[targetSlot];

    int slot = -1;
    for (int j = 0; j < data::MaxProjectiles; j++) {
        if (!state.projectiles[j].active) { slot = j; break; }
    }
    if (slot < 0) return;

    float dx = target.position.x - attacker.position.x;
    float dy = target.position.y - attacker.position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    bool aa = attacker.type == data::UnitType::AA;
    float speed = aa ? data::ShellSpeedAA : data::ShellSpeedRocketeer;
    int flightTicks = static_cast<int>(distance / speed / static_cast<float>(data::TickDelta) + 0.5f);
    if (flightTicks < 1) flightTicks = 1;

    int muzzleCount = data::MuzzleCount(attacker.type);
    int total = ComputeDamage(attacker, target);
    int perShell = muzzleCount > 1 ? total / muzzleCount : total;
    if (perShell < 1) perShell = 1;

    Projectile &p = state.projectiles[slot];
    p.active = true;
    p.arc = aa;
    p.team = attacker.team;
    p.attackerSlot = attackerIndex;
    p.muzzleIndex = muzzleIndex;
    p.targetSlot = targetSlot;
    p.origin = attacker.position;
    p.target = target.position;
    p.launchTick = state.tick;
    p.impactTick = state.tick + static_cast<std::uint64_t>(flightTicks);
    p.damage = perShell;
}

int AcquireTarget(const GameState &state, int index) {
    const Entity &self = state.entities[index];
    data::UnitStats stats = data::UnitStatsOf(self.type);
    if (self.type == data::UnitType::Engineer) return AcquireAlly(state, index);
    if (stats.baseDamage <= 0) return -1;

    data::Offset here = {self.col, self.row};
    int best = -1;
    int bestDistance = INT_MAX;
    for (int j = 0; j < data::MaxEntities; j++) {
        if (j == index) continue;
        const Entity &other = state.entities[j];
        if (!other.active || other.team == self.team) continue;
        if (!CanDamage(self, other)) continue;
        if (other.kind == EntityKind::Wall && !WallBlocksPath(self, other)) continue;
        int distance = data::HexDistance(here, {other.col, other.row});
        if (distance <= stats.attackRange && distance < bestDistance) {
            bestDistance = distance;
            best = j;
        }
    }
    return best;
}

bool CellIsWater(const Map &map, int col, int row) {
    if (!map.InBounds(col, row)) return true;
    return map.At(col, row) == data::TileType::RedBorder;
}

bool FootprintOnLand(const Map &map, data::Vec2 center, float radius) {
    data::Vec2 samples[5] = {
        center,
        {center.x + radius, center.y},
        {center.x - radius, center.y},
        {center.x, center.y + radius},
        {center.x, center.y - radius},
    };
    for (data::Vec2 point : samples) {
        data::Offset cell = data::CellFromLogic(point);
        if (CellIsWater(map, cell.col, cell.row)) return false;
    }
    return true;
}

void RefreshCell(Entity &entity) {
    data::Offset cell = data::CellFromLogic(entity.position);
    entity.col = cell.col;
    entity.row = cell.row;
}

void AdvanceUnit(Entity &entity, const Map &map, float dt) {
    int dir = entity.team == data::Team::Top ? 1 : -1;

    data::TileType tile = map.InBounds(entity.col, entity.row)
        ? map.At(entity.col, entity.row)
        : data::TileType::Field;
    float modifier = data::TileConfigOf(tile).moveModifier;
    float effective = data::EffectiveMoveModifier(entity.type, modifier);
    float step = data::UnitStatsOf(entity.type).moveSpeed * effective * dt;

    if (entity.type == data::UnitType::Plane) {
        entity.position.y += static_cast<float>(dir) * step;
        RefreshCell(entity);
        return;
    }

    float radius = data::UnitStatsOf(entity.type).footprint;
    float forwardY = entity.position.y + static_cast<float>(dir) * step;

    if (FootprintOnLand(map, {entity.position.x, forwardY}, radius)) {
        entity.position.y = forwardY;
        RefreshCell(entity);
        return;
    }

    for (int side : {1, -1}) {
        data::Vec2 diagonal = {entity.position.x + static_cast<float>(side) * step,
                               entity.position.y + static_cast<float>(dir) * step * 0.5f};
        if (FootprintOnLand(map, diagonal, radius)) {
            entity.position = diagonal;
            RefreshCell(entity);
            return;
        }
    }

    for (int side : {1, -1}) {
        data::Vec2 lateral = {entity.position.x + static_cast<float>(side) * step, entity.position.y};
        if (FootprintOnLand(map, lateral, radius)) {
            entity.position = lateral;
            RefreshCell(entity);
            return;
        }
    }
}

}

void Simulation::Init(GameState &state, const Map &map) {
    map_ = &map;

    int topMax = -1;
    int bottomMin = std::numeric_limits<int>::max();
    for (int row = 0; row < MapRows; row++) {
        for (int col = 0; col < MapCols; col++) {
            if (map.At(col, row) != data::TileType::Base) continue;
            if (row < MapRows / 2) {
                if (row > topMax) topMax = row;
            } else {
                if (row < bottomMin) bottomMin = row;
            }
        }
    }

    enemyBaseRow_[data::TeamIndex(data::Team::Top)] = bottomMin;
    enemyBaseRow_[data::TeamIndex(data::Team::Bottom)] = topMax;
    int spawnRow[2];
    spawnRow[data::TeamIndex(data::Team::Top)] = topMax + 1;
    spawnRow[data::TeamIndex(data::Team::Bottom)] = bottomMin - 1;

    state.tick = 0;
    state.winner = -1;
    state.resource[0] = 0.0f;
    state.resource[1] = 0.0f;
    for (int i = 0; i < data::MaxEntities; i++) {
        state.entities[i].active = false;
    }
    for (int j = 0; j < data::MaxProjectiles; j++) {
        state.projectiles[j].active = false;
    }

    int count = 0;
    auto addEntity = [&](EntityKind kind, data::UnitType type, data::Team team, int col, int row, int hp) {
        Entity &entity = state.entities[count];
        entity.active = true;
        entity.id = static_cast<std::uint32_t>(count);
        entity.kind = kind;
        entity.type = type;
        entity.team = team;
        entity.col = col;
        entity.row = row;
        entity.position = data::CellToLogic(col, row);
        entity.hp = hp;
        entity.targetSlot = -1;
        entity.attackCooldown = 0.0f;
        entity.burstIndex = 0;
        count++;
    };

    data::Team teams[2] = {data::Team::Top, data::Team::Bottom};
    for (data::Team team : teams) {
        for (int i = 0; i < data::UnitTypeCount; i++) {
            int col = i * (MapCols - 1) / (data::UnitTypeCount - 1);
            auto type = static_cast<data::UnitType>(i);
            addEntity(EntityKind::Unit, type, team, col, spawnRow[data::TeamIndex(team)], data::UnitStatsOf(type).hp);
        }
    }

    for (int row = 0; row < MapRows; row++) {
        for (int col = 0; col < MapCols; col++) {
            data::TileType tile = map.At(col, row);
            data::Team owner = row < MapRows / 2 ? data::Team::Top : data::Team::Bottom;
            if (tile == data::TileType::Wall) {
                addEntity(EntityKind::Wall, data::UnitType::Infantry, owner, col, row, data::WallHp);
            }
        }
    }

    int baseRow[2] = {topMax, bottomMin};
    for (data::Team team : teams) {
        int row = baseRow[data::TeamIndex(team)];
        int minCol = MapCols, maxCol = -1;
        for (int col = 0; col < MapCols; col++) {
            if (map.At(col, row) == data::TileType::Base) {
                if (col < minCol) minCol = col;
                if (col > maxCol) maxCol = col;
            }
        }
        int centerCol = (minCol + maxCol) / 2;
        addEntity(EntityKind::Base, data::UnitType::Infantry, team, centerCol, row, data::BaseHp);
    }

    state.entityCount = count;
}

void Simulation::Step(GameState &state, float dt) {
    if (state.winner >= 0) return;
    if (map_ == nullptr) return;
    state.tick++;

    float seconds = static_cast<float>(state.tick) * static_cast<float>(data::TickDelta);
    float regen = data::RegenPerSec(seconds) * dt;
    for (int t = 0; t < 2; t++) {
        state.resource[t] += regen;
        if (state.resource[t] > data::ResourceCap) state.resource[t] = data::ResourceCap;
    }

    for (int i = 0; i < data::MaxEntities; i++) {
        Entity &entity = state.entities[i];
        if (!entity.active || entity.kind != EntityKind::Unit) continue;

        int target = AcquireTarget(state, i);
        entity.targetSlot = target;

        bool isEngineer = entity.type == data::UnitType::Engineer;
        if (target >= 0 && !isEngineer) {
            entity.attackCooldown -= dt;
            if (entity.attackCooldown <= 0.0f) {
                if (UsesProjectile(entity.type)) {
                    FireProjectile(state, i, target, entity.burstIndex);
                    entity.burstIndex++;
                    if (entity.burstIndex >= data::MuzzleCount(entity.type)) {
                        entity.burstIndex = 0;
                        entity.attackCooldown = data::UnitStatsOf(entity.type).attackInterval;
                    } else {
                        entity.attackCooldown = data::BurstDelay;
                    }
                } else {
                    Entity &victim = state.entities[target];
                    victim.hp -= ComputeDamage(entity, victim);
                    entity.attackCooldown = data::UnitStatsOf(entity.type).attackInterval;
                }
            }
            continue;
        }

        int baseRow = enemyBaseRow_[data::TeamIndex(entity.team)];
        bool arrived = entity.team == data::Team::Top ? entity.row >= baseRow : entity.row <= baseRow;
        if (!arrived) AdvanceUnit(entity, *map_, dt);
    }

    for (int j = 0; j < data::MaxProjectiles; j++) {
        Projectile &p = state.projectiles[j];
        if (!p.active || state.tick < p.impactTick) continue;
        Entity &victim = state.entities[p.targetSlot];
        if (victim.active) victim.hp -= p.damage;
        p.active = false;
    }

    for (int i = 0; i < data::MaxEntities; i++) {
        Entity &entity = state.entities[i];
        if (!entity.active || entity.hp > 0) continue;
        entity.active = false;
        if (entity.kind == EntityKind::Base) {
            state.winner = entity.team == data::Team::Top ? 1 : 0;
        }
    }
}

}
