#include "debug/projectile_test_scene.h"

#include <cmath>

#include "raymath.h"

#include "data/economy/economy.h"
#include "data/sim/sim_config.h"
#include "data/space/hex.h"
#include "data/time/time_config.h"
#include "data/unit/damage.h"
#include "data/unit/unit.h"

namespace debug
{
namespace
{
    const data::UnitType Attackers[] = {data::UnitType::AA, data::UnitType::Rocketeer, data::UnitType::Tank,
                                        data::UnitType::Infantry, data::UnitType::Plane, data::UnitType::Engineer};
    const char* AttackerNames[] = {"AA", "Rocketeer", "Tank", "Infantry", "Plane", "Engineer"};

    bool UsesProjectile(data::UnitType type)
    {
        return type == data::UnitType::AA || type == data::UnitType::Rocketeer || type == data::UnitType::Tank;
    }

    const data::UnitType Targets[] = {
        data::UnitType::Infantry, data::UnitType::Rocketeer, data::UnitType::Engineer,
        data::UnitType::AA, data::UnitType::Tank, data::UnitType::Plane,
    };
    const char* TargetNames[] = {"Infantry", "Rocketeer", "Engineer", "AA", "Tank", "Plane"};

    constexpr float AttackerY = -150.0f;
    constexpr float TargetY = 150.0f;
}

int ProjectileTestScene::AttackerCount() const { return 6; }
int ProjectileTestScene::TargetCount() const { return 6; }
const char* ProjectileTestScene::AttackerName(int index) const { return AttackerNames[index]; }
const char* ProjectileTestScene::TargetName(int index) const { return TargetNames[index]; }

void ProjectileTestScene::UpdateCamera()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        Vector2 delta = GetMouseDelta();
        yaw_ -= delta.x * 0.3f;
        pitch_ += delta.y * 0.3f;
        pitch_ = Clamp(pitch_, -85.0f, 85.0f);
    }
    distance_ -= GetMouseWheelMove() * 0.5f;
    distance_ = Clamp(distance_, 2.0f, 30.0f);
}

Camera3D ProjectileTestScene::Camera() const
{
    float yr = yaw_ * DEG2RAD;
    float pr = pitch_ * DEG2RAD;
    Vector3 target = {0.0f, 1.0f, 0.0f};
    Vector3 position = {
        target.x + distance_ * std::cos(pr) * std::sin(yr),
        target.y + distance_ * std::sin(pr),
        target.z + distance_ * std::cos(pr) * std::cos(yr),
    };
    Camera3D camera = {};
    camera.position = position;
    camera.target = target;
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}

void ProjectileTestScene::Rebuild()
{
    current_ = logic::GameState{};
    current_.winner = -1;
    for (int i = 0; i < data::MaxEntities; i++) current_.entities[i].active = false;
    for (int j = 0; j < data::MaxProjectiles; j++) current_.projectiles[j].active = false;

    auto add = [&](int slot, data::UnitType type, data::Team team, data::Vec2 pos) {
        logic::Entity& e = current_.entities[slot];
        e.active = true;
        e.id = static_cast<std::uint32_t>(slot);
        e.kind = logic::EntityKind::Unit;
        e.type = type;
        e.team = team;
        e.position = pos;
        data::Offset cell = data::CellFromLogic(pos);
        e.col = cell.col;
        e.row = cell.row;
        e.hp = data::UnitStatsOf(type).hp;
        e.targetSlot = -1;
        e.attackCooldown = 0.0f;
        e.burstIndex = 0;
    };

    add(0, Attackers[attacker_], data::Team::Top, {0.0f, AttackerY});
    add(1, Targets[target_], data::Team::Bottom, {0.0f, TargetY});
    current_.entities[0].targetSlot = 1;
    if (Targets[target_] != data::UnitType::Plane) current_.entities[1].targetSlot = 0;
    current_.entityCount = 2;

    previous_ = current_;
    accumulator_ = 0.0;
    builtAttacker_ = attacker_;
    builtTarget_ = target_;
}

void ProjectileTestScene::Step()
{
    logic::Entity& a = current_.entities[0];
    logic::Entity& t = current_.entities[1];
    t.hp = data::UnitStatsOf(t.type).hp;

    a.attackCooldown -= static_cast<float>(data::TickDelta);
    if (a.attackCooldown <= 0.0f && UsesProjectile(a.type))
    {
        int slot = -1;
        for (int j = 0; j < data::MaxProjectiles; j++)
        {
            if (!current_.projectiles[j].active) { slot = j; break; }
        }
        if (slot >= 0)
        {
            float dx = t.position.x - a.position.x;
            float dy = t.position.y - a.position.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            bool aa = a.type == data::UnitType::AA;
            float speed = aa ? data::ShellSpeedAA : data::ShellSpeedRocketeer;
            int flightTicks = static_cast<int>(distance / speed / static_cast<float>(data::TickDelta) + 0.5f);
            if (flightTicks < 1) flightTicks = 1;

            logic::Projectile& p = current_.projectiles[slot];
            p.active = true;
            p.arc = aa;
            p.team = a.team;
            p.attackerSlot = 0;
            p.muzzleIndex = a.burstIndex;
            p.targetSlot = 1;
            p.origin = a.position;
            p.target = t.position;
            p.launchTick = current_.tick;
            p.impactTick = current_.tick + static_cast<std::uint64_t>(flightTicks);
            p.damage = static_cast<int>(data::UnitStatsOf(a.type).baseDamage *
                                        data::DamageMultiplier(a.type, t.type));
        }
        a.burstIndex++;
        if (a.burstIndex >= data::MuzzleCount(a.type))
        {
            a.burstIndex = 0;
            a.attackCooldown = data::UnitStatsOf(a.type).attackInterval;
        }
        else
        {
            a.attackCooldown = data::BurstDelay;
        }
    }
    else if (a.attackCooldown <= 0.0f && data::UnitStatsOf(a.type).baseDamage > 0)
    {
        int slot = -1;
        for (int j = 0; j < data::MaxBeams; j++)
        {
            if (!current_.beams[j].active) { slot = j; break; }
        }
        if (slot >= 0)
        {
            logic::Beam& beam = current_.beams[slot];
            beam.active = true;
            beam.attackerSlot = 0;
            beam.targetSlot = 1;
            beam.muzzleIndex = a.burstIndex;
            beam.startTick = current_.tick;
        }
        t.hp -= static_cast<int>(data::UnitStatsOf(a.type).baseDamage * data::DamageMultiplier(a.type, t.type));
        a.burstIndex = (a.burstIndex + 1) % data::MuzzleCount(a.type);
        a.attackCooldown = data::UnitStatsOf(a.type).attackInterval;
    }
    else if (a.attackCooldown <= 0.0f && a.type == data::UnitType::Engineer)
    {
        int slot = -1;
        for (int j = 0; j < data::MaxHealPulses; j++)
        {
            if (!current_.healPulses[j].active) { slot = j; break; }
        }
        if (slot >= 0)
        {
            logic::HealPulse& pulse = current_.healPulses[slot];
            pulse.active = true;
            pulse.center = a.position;
            pulse.radius = data::EngineerHealPulseRadius;
            pulse.startTick = current_.tick;
        }
        a.attackCooldown = data::EngineerHealPulseInterval;
    }

    for (int j = 0; j < data::MaxProjectiles; j++)
    {
        logic::Projectile& p = current_.projectiles[j];
        if (!p.active || current_.tick < p.impactTick) continue;
        t.hp -= p.damage;
        p.active = false;
    }

    std::uint64_t beamTicks = static_cast<std::uint64_t>(data::BeamSeconds / static_cast<float>(data::TickDelta))
        + 1;
    for (int j = 0; j < data::MaxBeams; j++)
    {
        logic::Beam& beam = current_.beams[j];
        if (beam.active && current_.tick - beam.startTick > beamTicks) beam.active = false;
    }

    std::uint64_t waveTicks =
        static_cast<std::uint64_t>(data::HealWaveSeconds / static_cast<float>(data::TickDelta)) + 1;
    for (int j = 0; j < data::MaxHealPulses; j++)
    {
        logic::HealPulse& pulse = current_.healPulses[j];
        if (pulse.active && current_.tick - pulse.startTick > waveTicks) pulse.active = false;
    }
}

void ProjectileTestScene::Update(float dt)
{
    if (attacker_ != builtAttacker_ || target_ != builtTarget_) Rebuild();

    accumulator_ += dt;
    int steps = 0;
    while (accumulator_ >= data::TickDelta && steps < data::MaxTicksPerFrame)
    {
        previous_ = current_;
        current_.tick++;
        Step();
        accumulator_ -= data::TickDelta;
        steps++;
    }
    alpha_ = static_cast<float>(accumulator_ / data::TickDelta);
}
}
