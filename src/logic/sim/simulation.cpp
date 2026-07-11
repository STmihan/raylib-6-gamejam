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
#include "logic/path.h"

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

void ApplyHit(Entity &victim, int damage) {
    if (damage <= 0) return;
    if (victim.kind == EntityKind::Unit && victim.armorHits > 0) {
        victim.armorHits--;
        return;
    }
    victim.hp -= damage;
}

void SpawnBeam(GameState &state, int attackerIndex, int targetSlot, int muzzleIndex) {
    for (int j = 0; j < data::MaxBeams; j++) {
        if (state.beams[j].active) continue;
        state.beams[j].active = true;
        state.beams[j].attackerSlot = attackerIndex;
        state.beams[j].targetSlot = targetSlot;
        state.beams[j].muzzleIndex = muzzleIndex;
        state.beams[j].startTick = state.tick;
        return;
    }
}

void SpawnHealPulse(GameState &state, data::Vec2 center, int radius) {
    for (int j = 0; j < data::MaxHealPulses; j++) {
        if (state.healPulses[j].active) continue;
        state.healPulses[j].active = true;
        state.healPulses[j].center = center;
        state.healPulses[j].radius = radius;
        state.healPulses[j].startTick = state.tick;
        return;
    }
}

void ApplyHealArea(GameState &state, data::Team team, int col, int row, int radius, bool fractional, float value) {
    data::Offset here = {col, row};
    for (int j = 0; j < data::MaxEntities; j++) {
        Entity &ally = state.entities[j];
        if (!ally.active || ally.kind != EntityKind::Unit || ally.team != team) continue;
        if (data::HexDistance(here, {ally.col, ally.row}) > radius) continue;
        int maxHp = data::UnitStatsOf(ally.type).hp;
        int heal = fractional ? static_cast<int>(value * static_cast<float>(maxHp)) : static_cast<int>(value);
        ally.hp += heal;
        if (ally.hp > maxHp) ally.hp = maxHp;
    }
    SpawnHealPulse(state, data::CellToLogic(col, row), radius);
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

int AcquireBaseTarget(const GameState &state, int index, int range) {
    const Entity &self = state.entities[index];
    data::Offset here = {self.col, self.row};
    int best = -1;
    int bestDistance = INT_MAX;
    for (int j = 0; j < data::MaxEntities; j++) {
        const Entity &other = state.entities[j];
        if (!other.active || other.kind != EntityKind::Unit || other.team == self.team) continue;
        int distance = data::HexDistance(here, {other.col, other.row});
        if (distance <= range && distance < bestDistance) {
            bestDistance = distance;
            best = j;
        }
    }
    return best;
}

int AcquireAirTarget(const GameState &state, int index) {
    const Entity &self = state.entities[index];
    data::Offset here = {self.col, self.row};
    int best = -1;
    int bestDistance = INT_MAX;
    for (int j = 0; j < data::MaxEntities; j++) {
        if (j == index) continue;
        const Entity &other = state.entities[j];
        if (!other.active || other.team == self.team) continue;
        if (other.type != data::UnitType::Plane) continue;
        if (!CanDamage(self, other)) continue;
        int distance = data::HexDistance(here, {other.col, other.row});
        if (distance <= data::UnitStatsOf(self.type).attackRange && distance < bestDistance) {
            bestDistance = distance;
            best = j;
        }
    }
    return best;
}

bool ForcedTargetValid(const GameState &state, int index, int target) {
    if (target < 0 || target >= data::MaxEntities) return false;
    const Entity &self = state.entities[index];
    const Entity &t = state.entities[target];
    if (!t.active || t.team == self.team) return false;
    return CanDamage(self, t);
}

bool InAttackRange(const GameState &state, int index, int target) {
    const Entity &self = state.entities[index];
    const Entity &t = state.entities[target];
    int dist = data::HexDistance({self.col, self.row}, {t.col, t.row});
    return dist <= data::UnitStatsOf(self.type).attackRange;
}

bool StillValid(const GameState &state, int index, int target) {
    if (target < 0 || target >= data::MaxEntities) return false;
    const Entity &self = state.entities[index];
    const Entity &t = state.entities[target];
    if (!t.active || t.team == self.team) return false;
    if (!CanDamage(self, t)) return false;
    if (t.kind == EntityKind::Wall && !WallBlocksPath(self, t)) return false;
    int dist = data::HexDistance({self.col, self.row}, {t.col, t.row});
    return dist <= data::UnitStatsOf(self.type).attackRange;
}

void RefreshCell(Entity &entity) {
    data::Offset cell = data::CellFromLogic(entity.position);
    entity.col = cell.col;
    entity.row = cell.row;
}

void MoveTowardFree(Entity &entity, float dt, data::Vec2 dest) {
    float step = data::UnitStatsOf(entity.type).moveSpeed * dt;
    float dx = dest.x - entity.position.x;
    float dy = dest.y - entity.position.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len <= step || len < 1.0f) {
        entity.position = dest;
    } else {
        entity.position.x += dx / len * step;
        entity.position.y += dy / len * step;
    }
    RefreshCell(entity);
}

void MovePlane(Entity &entity, const Map &map, float dt, data::Vec2 basePos) {
    int dir = entity.team == data::Team::Top ? 1 : -1;
    float step = data::UnitStatsOf(entity.type).moveSpeed * dt;
    data::TileType tile = map.InBounds(entity.col, entity.row) ? map.At(entity.col, entity.row)
                                                               : data::TileType::Field;
    bool enemyHalf = entity.team == data::Team::Bottom ? entity.row < MapRows / 2 : entity.row >= MapRows / 2;
    if (tile == data::TileType::ConcreteRoad && enemyHalf) {
        MoveTowardFree(entity, dt, basePos);
        return;
    }
    entity.position.y += static_cast<float>(dir) * step;
    RefreshCell(entity);
}

void StepGroundMove(Entity &entity, const Map &map, const int *occupant, int selfIndex, float dt, int goalCol,
                    int goalRow, int stopRange) {
    if (data::HexDistance({entity.col, entity.row}, {goalCol, goalRow}) <= stopRange) {
        entity.pathCol = -1;
        return;
    }

    entity.repathTimer -= dt;
    bool reachedStep = entity.pathCol == entity.col && entity.pathRow == entity.row;
    bool stepBlocked = entity.pathCol >= 0 && !Passable(map, occupant, selfIndex, entity.pathCol, entity.pathRow);
    if (entity.pathCol < 0 || reachedStep || stepBlocked || entity.repathTimer <= 0.0f) {
        int nc = 0;
        int nr = 0;
        if (FindStep(map, occupant, selfIndex, entity.col, entity.row, goalCol, goalRow, stopRange, nc, nr)) {
            entity.pathCol = nc;
            entity.pathRow = nr;
        } else {
            entity.pathCol = -1;
        }
        entity.repathTimer = data::PathRepathSeconds;
    }
    if (entity.pathCol < 0) return;

    data::Vec2 dest = data::CellToLogic(entity.pathCol, entity.pathRow);
    data::TileType tile = map.InBounds(entity.col, entity.row) ? map.At(entity.col, entity.row)
                                                               : data::TileType::Field;
    float modifier = data::EffectiveMoveModifier(entity.type, data::TileConfigOf(tile).moveModifier);
    float step = data::UnitStatsOf(entity.type).moveSpeed * modifier * dt;
    float dx = dest.x - entity.position.x;
    float dy = dest.y - entity.position.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len <= step || len < 1.0f) {
        entity.position = dest;
    } else {
        entity.position.x += dx / len * step;
        entity.position.y += dy / len * step;
    }
    RefreshCell(entity);
}

int SelectTarget(GameState &state, int index) {
    Entity &entity = state.entities[index];

    bool isEngineer = entity.type == data::UnitType::Engineer;

    if (entity.forcedTarget >= 0) {
        const Entity &ft = state.entities[entity.forcedTarget];
        bool ok = isEngineer
            ? (ft.active && ft.kind == EntityKind::Unit && ft.team == entity.team)
            : ForcedTargetValid(state, index, entity.forcedTarget);
        if (!ok) entity.forcedTarget = -1;
    }

    if (isEngineer) {
        return AcquireTarget(state, index);
    }
    if (entity.forcedTarget >= 0) {
        return InAttackRange(state, index, entity.forcedTarget) ? entity.forcedTarget : -1;
    }

    int cur = entity.targetSlot;
    if (entity.type == data::UnitType::Plane) {
        int air = AcquireAirTarget(state, index);
        bool curIsPlane = cur >= 0 && state.entities[cur].active
            && state.entities[cur].type == data::UnitType::Plane;
        if (air >= 0) return (curIsPlane && StillValid(state, index, cur)) ? cur : air;
        if (StillValid(state, index, cur)) return cur;
        return AcquireTarget(state, index);
    }

    if (StillValid(state, index, cur)) return cur;
    return AcquireTarget(state, index);
}

}

void Simulation::Init(GameState &state, const Map &map) {
    map_ = &map;

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
    for (int j = 0; j < data::MaxHealPulses; j++) {
        state.healPulses[j].active = false;
    }
    for (int j = 0; j < data::MaxBeams; j++) {
        state.beams[j].active = false;
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
        entity.forcedTarget = -1;
        entity.attackCooldown = 0.0f;
        entity.burstIndex = 0;
        entity.deployTimer = 0.0f;
        entity.hasMoveOrder = false;
        entity.moveTarget = {};
        entity.armorHits = data::UnitStatsOf(type).armorHits;
        entity.armorMax = entity.armorHits;
        entity.pathCol = -1;
        entity.pathRow = -1;
        entity.repathTimer = 0.0f;
        count++;
    };

    data::Team teams[2] = {data::Team::Top, data::Team::Bottom};

    for (int row = 0; row < MapRows; row++) {
        for (int col = 0; col < MapCols; col++) {
            data::TileType tile = map.At(col, row);
            data::Team owner = row < MapRows / 2 ? data::Team::Top : data::Team::Bottom;
            if (tile == data::TileType::Wall) {
                addEntity(EntityKind::Wall, data::UnitType::Infantry, owner, col, row, data::WallHp);
            }
        }
    }

    int baseCol[2] = {0, 0};
    int baseRow[2] = {0, 0};
    for (data::Team team : teams) {
        int idx = data::TeamIndex(team);
        int minCol = MapCols, maxCol = -1, minRow = MapRows, maxRow = -1;
        for (int row = 0; row < MapRows; row++) {
            for (int col = 0; col < MapCols; col++) {
                if (map.At(col, row) != data::TileType::Base) continue;
                data::Team owner = row < MapRows / 2 ? data::Team::Top : data::Team::Bottom;
                if (owner != team) continue;
                if (col < minCol) minCol = col;
                if (col > maxCol) maxCol = col;
                if (row < minRow) minRow = row;
                if (row > maxRow) maxRow = row;
            }
        }
        int centerCol = (minCol + maxCol) / 2;
        int centerRow = (minRow + maxRow) / 2;
        baseCol[idx] = centerCol;
        baseRow[idx] = centerRow;
        addEntity(EntityKind::Base, data::UnitType::Infantry, team, centerCol, centerRow, data::BaseHp);
    }

    int topIdx = data::TeamIndex(data::Team::Top);
    int botIdx = data::TeamIndex(data::Team::Bottom);
    enemyBaseCol_[topIdx] = baseCol[botIdx];
    enemyBaseRow_[topIdx] = baseRow[botIdx];
    enemyBaseCol_[botIdx] = baseCol[topIdx];
    enemyBaseRow_[botIdx] = baseRow[topIdx];
    enemyBasePos_[topIdx] = data::CellToLogic(baseCol[botIdx], baseRow[botIdx]);
    enemyBasePos_[botIdx] = data::CellToLogic(baseCol[topIdx], baseRow[topIdx]);

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

    occupant_.fill(-1);
    for (int i = 0; i < data::MaxEntities; i++) {
        Entity &e = state.entities[i];
        if (!e.active || e.kind != EntityKind::Unit || e.type == data::UnitType::Plane) continue;
        if (map_->InBounds(e.col, e.row)) occupant_[static_cast<std::size_t>(e.row) * MapCols + e.col] = i;
    }

    for (int i = 0; i < data::MaxEntities; i++) {
        Entity &entity = state.entities[i];
        if (!entity.active || entity.kind != EntityKind::Unit) continue;

        if (entity.deployTimer > 0.0f) {
            entity.deployTimer -= dt;
            entity.targetSlot = -1;
            continue;
        }

        bool stationary = data::UnitStatsOf(entity.type).stationary;

        if (stationary && entity.hasMoveOrder) {
            entity.targetSlot = -1;
            data::Offset dest = data::CellFromLogic(entity.moveTarget);
            if (entity.col == dest.col && entity.row == dest.row) {
                entity.hasMoveOrder = false;
            } else {
                StepGroundMove(entity, *map_, occupant_.data(), i, dt, dest.col, dest.row, 0);
            }
            continue;
        }

        bool isEngineer = entity.type == data::UnitType::Engineer;
        int target = SelectTarget(state, i);
        entity.targetSlot = target;

        if (isEngineer) {
            entity.attackCooldown -= dt;
            if (entity.attackCooldown <= 0.0f) {
                ApplyHealArea(state, entity.team, entity.col, entity.row, data::EngineerHealPulseRadius, false,
                              static_cast<float>(data::EngineerHealPulseAmount));
                entity.attackCooldown = data::EngineerHealPulseInterval;
            }
        }

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
                    int muzzle = entity.burstIndex;
                    ApplyHit(victim, ComputeDamage(entity, victim));
                    SpawnBeam(state, i, target, muzzle);
                    entity.burstIndex = (entity.burstIndex + 1) % data::MuzzleCount(entity.type);
                    entity.attackCooldown = data::UnitStatsOf(entity.type).attackInterval;
                }
            }
            continue;
        }

        int teamIdx = data::TeamIndex(entity.team);

        if (entity.type == data::UnitType::Plane) {
            if (entity.forcedTarget >= 0) {
                MoveTowardFree(entity, dt, state.entities[entity.forcedTarget].position);
            } else {
                MovePlane(entity, *map_, dt, enemyBasePos_[teamIdx]);
            }
            continue;
        }

        if (stationary) continue;

        int goalCol;
        int goalRow;
        int stopRange = data::UnitStatsOf(entity.type).attackRange;
        if (entity.forcedTarget >= 0) {
            goalCol = state.entities[entity.forcedTarget].col;
            goalRow = state.entities[entity.forcedTarget].row;
        } else {
            goalCol = enemyBaseCol_[teamIdx];
            goalRow = enemyBaseRow_[teamIdx];
            if (stopRange < 2) stopRange = 2;
        }
        StepGroundMove(entity, *map_, occupant_.data(), i, dt, goalCol, goalRow, stopRange);
    }

    for (int i = 0; i < data::MaxEntities; i++) {
        Entity &base = state.entities[i];
        if (!base.active || base.kind != EntityKind::Base) continue;

        int target = AcquireBaseTarget(state, i, data::BaseTurretRange);
        base.targetSlot = target;
        if (target < 0) continue;

        base.attackCooldown -= dt;
        if (base.attackCooldown <= 0.0f) {
            ApplyHit(state.entities[target], data::BaseTurretDamage);
            SpawnBeam(state, i, target, base.burstIndex);
            base.burstIndex++;
            base.attackCooldown = data::BaseTurretInterval;
        }
    }

    for (int j = 0; j < data::MaxProjectiles; j++) {
        Projectile &p = state.projectiles[j];
        if (!p.active || state.tick < p.impactTick) continue;
        Entity &victim = state.entities[p.targetSlot];
        if (victim.active) ApplyHit(victim, p.damage);
        p.active = false;
    }

    std::uint64_t waveTicks = static_cast<std::uint64_t>(data::HealWaveSeconds / static_cast<float>(data::TickDelta))
        + 1;
    for (int j = 0; j < data::MaxHealPulses; j++) {
        HealPulse &pulse = state.healPulses[j];
        if (pulse.active && state.tick - pulse.startTick > waveTicks) pulse.active = false;
    }

    std::uint64_t beamTicks = static_cast<std::uint64_t>(data::BeamSeconds / static_cast<float>(data::TickDelta))
        + 1;
    for (int j = 0; j < data::MaxBeams; j++) {
        Beam &beam = state.beams[j];
        if (beam.active && state.tick - beam.startTick > beamTicks) beam.active = false;
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

int Simulation::Deploy(GameState &state, data::UnitType type, data::Team team, int col, int row) {
    int slot = -1;
    for (int i = 0; i < data::MaxEntities; i++) {
        if (!state.entities[i].active) { slot = i; break; }
    }
    if (slot < 0) return -1;

    Entity &e = state.entities[slot];
    e.active = true;
    e.id = static_cast<std::uint32_t>(slot);
    e.kind = EntityKind::Unit;
    e.type = type;
    e.team = team;
    e.col = col;
    e.row = row;
    e.position = data::CellToLogic(col, row);
    e.hp = data::UnitStatsOf(type).hp;
    e.targetSlot = -1;
    e.forcedTarget = -1;
    e.attackCooldown = 0.0f;
    e.burstIndex = 0;
    e.deployTimer = data::DeployFreezeSeconds;
    e.hasMoveOrder = false;
    e.moveTarget = {};
    e.armorHits = data::UnitStatsOf(type).armorHits;
    e.armorMax = e.armorHits;
    e.pathCol = -1;
    e.pathRow = -1;
    e.repathTimer = 0.0f;
    if (slot >= state.entityCount) state.entityCount = slot + 1;

    if (type == data::UnitType::Engineer) {
        e.attackCooldown = data::EngineerHealPulseInterval;
        ApplyHealArea(state, team, col, row, data::EngineerHealDeployRadius, true, data::EngineerHealDeployFraction);
    }
    return slot;
}

void Simulation::CommandTarget(GameState &state, int slot, int targetSlot) {
    if (slot < 0 || slot >= data::MaxEntities) return;
    Entity &e = state.entities[slot];
    if (!e.active || e.kind != EntityKind::Unit) return;
    e.forcedTarget = targetSlot;
    e.hasMoveOrder = false;
}

void Simulation::CommandMove(GameState &state, int slot, int col, int row) {
    if (slot < 0 || slot >= data::MaxEntities) return;
    Entity &e = state.entities[slot];
    if (!e.active || e.kind != EntityKind::Unit) return;
    e.moveTarget = data::CellToLogic(col, row);
    e.hasMoveOrder = true;
    e.forcedTarget = -1;
}

}
