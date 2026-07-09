#include "view/prefab/plane_orbit.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "raymath.h"

#include "data/sim/sim_config.h"
#include "view/space/interpolator.h"
#include "view/space/orientation.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    constexpr float TwoPi = 6.2831853f;
    constexpr float PlaneClusterRadius = 4.0f;

    Vector2 EntityXZ(const logic::GameState& previous, const logic::GameState& current, int index, float alpha)
    {
        data::Vec2 logic = InterpolatedPosition(previous.entities[index], current.entities[index], alpha);
        Vector3 world = LogicToWorld(logic, 0.0f);
        return Vector2{world.x, world.z};
    }
}

Vector3 PlaneVisualPos(const logic::GameState& previous, const logic::GameState& current, int slot, float alpha,
                       float time, const PlaneOrbitParams& params, float& yawOut, float& rollOut)
{
    const logic::Entity& entity = current.entities[slot];
    Vector2 self = EntityXZ(previous, current, slot, alpha);

    if (entity.targetSlot < 0)
    {
        yawOut = TeamYaw(entity.team);
        rollOut = 0.0f;
        return Vector3{self.x, params.altitude, self.y};
    }

    const logic::Entity& target = current.entities[entity.targetSlot];
    bool airDuel = target.type == data::UnitType::Plane;

    std::vector<int> group;
    for (int j = 0; j < data::MaxEntities; j++)
    {
        const logic::Entity& other = current.entities[j];
        if (!other.active || other.type != data::UnitType::Plane || other.targetSlot < 0) continue;
        bool otherAir = current.entities[other.targetSlot].type == data::UnitType::Plane;
        if (airDuel != otherAir) continue;
        if (!airDuel && other.targetSlot != entity.targetSlot) continue;
        if (Vector2Distance(EntityXZ(previous, current, j, alpha), self) <= PlaneClusterRadius) group.push_back(j);
    }
    std::sort(group.begin(), group.end(), [&](int a, int b) {
        return current.entities[a].id < current.entities[b].id;
    });

    int count = static_cast<int>(group.size());
    if (count < 1) count = 1;
    int index = 0;
    for (int k = 0; k < static_cast<int>(group.size()); k++) if (group[k] == slot) index = k;

    Vector2 center;
    float radius;
    if (airDuel)
    {
        Vector2 sum = {0.0f, 0.0f};
        for (int j : group) sum = Vector2Add(sum, EntityXZ(previous, current, j, alpha));
        center = Vector2Scale(sum, 1.0f / static_cast<float>(count));
        radius = params.airRadius;
    }
    else
    {
        center = EntityXZ(previous, current, entity.targetSlot, alpha);
        radius = params.groundRadius;
    }

    float theta = time * params.speed + TwoPi * static_cast<float>(index) / static_cast<float>(count);
    Vector2 tangent = {-std::sin(theta), std::cos(theta)};
    yawOut = std::atan2(-tangent.x, -tangent.y) * RAD2DEG;
    rollOut = params.bank;
    return Vector3{center.x + std::cos(theta) * radius, params.altitude, center.y + std::sin(theta) * radius};
}
}
