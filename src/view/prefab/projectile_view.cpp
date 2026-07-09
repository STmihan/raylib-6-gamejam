#include "view/prefab/projectile_view.h"

#include <cmath>

#include "raylib.h"
#include "raymath.h"

#include "data/sim/sim_config.h"
#include "data/unit/unit.h"
#include "view/prefab/registries/muzzle_registry.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/vehicle_view.h"
#include "view/space/interpolator.h"
#include "view/space/orientation.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    Vector3 Bezier3(Vector3 a, Vector3 c, Vector3 b, float t)
    {
        float u = 1.0f - t;
        return Vector3{
            u * u * a.x + 2.0f * u * t * c.x + t * t * b.x,
            u * u * a.y + 2.0f * u * t * c.y + t * t * b.y,
            u * u * a.z + 2.0f * u * t * c.z + t * t * b.z,
        };
    }

    Matrix AttackerPartMatrix(const ModelRegistry& models, const logic::GameState& previous,
                              const logic::GameState& current, int slot, float alpha, float time,
                              const PlaneOrbitParams& orbit, Vector2 targetXZ)
    {
        const logic::Entity& a = current.entities[slot];
        data::Vec2 logic = InterpolatedPosition(previous.entities[slot], a, alpha);
        Vector3 ground = LogicToWorld(logic, 0.0f);

        if (a.type == data::UnitType::Plane)
        {
            float yaw;
            float roll;
            Vector3 pos = PlaneVisualPos(previous, current, slot, alpha, time, orbit, yaw, roll);
            Matrix m = MatrixRotateZ(roll * DEG2RAD);
            m = MatrixMultiply(m, MatrixRotateY(yaw * DEG2RAD));
            m = MatrixMultiply(m, MatrixTranslate(pos.x, pos.y, pos.z));
            return m;
        }

        float teamYaw = TeamYaw(a.team);
        if (data::UnitStatsOf(a.type).isVehicle)
        {
            float aim = YawTowards(Vector2{ground.x, ground.z}, targetXZ, teamYaw);
            float turretYaw = (aim - teamYaw) * DEG2RAD;
            return VehicleTurretMatrix(models, a.type, ground, teamYaw, turretYaw);
        }

        float yaw = YawTowards(Vector2{ground.x, ground.z}, targetXZ, teamYaw);
        Vector3 bodyPos = LogicToWorld(logic, models.UnitGroundOffset(a.type));
        Matrix matTransform = MatrixMultiply(MatrixRotateY(yaw * DEG2RAD),
                                             MatrixTranslate(bodyPos.x, bodyPos.y, bodyPos.z));
        return MatrixMultiply(models.UnitBody(a.type).transform, matTransform);
    }

    void DrawShell(const Model& model, Vector3 position, Vector3 direction, float scale)
    {
        Vector3 forward = {0.0f, 0.0f, -1.0f};
        float dot = Clamp(Vector3DotProduct(forward, direction), -1.0f, 1.0f);
        Vector3 axis = Vector3CrossProduct(forward, direction);
        float axisLen = Vector3Length(axis);
        Matrix rot = axisLen < 1e-4f ? (dot > 0.0f ? MatrixIdentity() : MatrixRotateY(PI))
                                     : MatrixRotate(Vector3Scale(axis, 1.0f / axisLen), std::acos(dot));
        Matrix m = MatrixScale(scale, scale, scale);
        m = MatrixMultiply(m, rot);
        m = MatrixMultiply(m, MatrixTranslate(position.x, position.y, position.z));
        for (int i = 0; i < model.meshCount; i++)
        {
            DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], m);
        }
    }
}

void ProjectileView::Draw(const ModelRegistry& models, const MuzzleRegistry& muzzles,
                          const logic::GameState& previous, const logic::GameState& current, float alpha,
                          const PlaneOrbitParams& orbit, float time) const
{
    float now = static_cast<float>(current.tick) - 1.0f + alpha;

    for (int j = 0; j < data::MaxProjectiles; j++)
    {
        const logic::Projectile& p = current.projectiles[j];
        if (!p.active) continue;

        float span = static_cast<float>(p.impactTick - p.launchTick);
        if (span < 1.0f) span = 1.0f;
        float t = (now - static_cast<float>(p.launchTick)) / span;
        if (t <= 0.0f || t >= 1.0f) continue;

        const logic::Entity& target = current.entities[p.targetSlot];
        Vector3 dest;
        if (target.active && target.type == data::UnitType::Plane)
        {
            float yaw;
            float roll;
            dest = PlaneVisualPos(previous, current, p.targetSlot, alpha, time, orbit, yaw, roll);
        }
        else if (target.active)
        {
            data::Vec2 logic = InterpolatedPosition(previous.entities[p.targetSlot], target, alpha);
            dest = LogicToWorld(logic, targetHeight_);
        }
        else
        {
            dest = LogicToWorld(p.target, targetHeight_);
        }
        Vector2 targetXZ = {dest.x, dest.z};

        Vector3 spawn;
        Vector3 launchDir;
        const logic::Entity& attacker = current.entities[p.attackerSlot];
        const Muzzle* mz = attacker.active ? muzzles.ForUnit(attacker.type, p.muzzleIndex) : nullptr;
        if (mz != nullptr)
        {
            Matrix part = AttackerPartMatrix(models, previous, current, p.attackerSlot, alpha, time, orbit,
                                             targetXZ);
            spawn = Vector3Transform(mz->pos, part);
            Vector3 tip = Vector3Transform(Vector3Add(mz->pos, mz->dir), part);
            launchDir = Vector3Normalize(Vector3Subtract(tip, spawn));
        }
        else
        {
            spawn = LogicToWorld(p.origin, launchHeight_);
            launchDir = Vector3Normalize(Vector3Subtract(dest, spawn));
        }

        float t2 = t + 0.02f > 1.0f ? 1.0f : t + 0.02f;
        Vector3 pos;
        Vector3 ahead;
        if (p.arc)
        {
            Vector3 ctrl = Vector3Add(spawn, Vector3Scale(launchDir, arcCurve_));
            ctrl.y += arcHeight_;
            pos = Bezier3(spawn, ctrl, dest, t);
            ahead = Bezier3(spawn, ctrl, dest, t2);
        }
        else
        {
            pos = Vector3Lerp(spawn, dest, t);
            ahead = Vector3Lerp(spawn, dest, t2);
        }

        Vector3 direction = Vector3Subtract(ahead, pos);
        direction = Vector3Length(direction) < 1e-5f ? launchDir : Vector3Normalize(direction);
        DrawShell(models.Shell(), pos, direction, shellScale_);
    }
}

void ProjectileView::DrawMuzzleGizmos(const ModelRegistry& models, const MuzzleRegistry& muzzles,
                                      const logic::GameState& previous, const logic::GameState& current,
                                      float alpha, const PlaneOrbitParams& orbit) const
{
    float time = static_cast<float>(GetTime());

    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& u = current.entities[i];
        if (!u.active || u.kind != logic::EntityKind::Unit) continue;
        int muzzleCount = data::MuzzleCount(u.type);
        if (muzzleCount <= 0) continue;

        int aimSlot = u.targetSlot;
        if (aimSlot < 0)
        {
            for (int j = 0; j < data::MaxEntities; j++)
            {
                if (j != i && current.entities[j].active && current.entities[j].kind == logic::EntityKind::Unit)
                {
                    aimSlot = j;
                    break;
                }
            }
        }

        Vector2 targetXZ;
        if (aimSlot >= 0)
        {
            data::Vec2 tl = InterpolatedPosition(previous.entities[aimSlot], current.entities[aimSlot], alpha);
            Vector3 tw = LogicToWorld(tl, 0.0f);
            targetXZ = Vector2{tw.x, tw.z};
        }
        else
        {
            Vector3 uw = LogicToWorld(InterpolatedPosition(previous.entities[i], u, alpha), 0.0f);
            targetXZ = Vector2{uw.x, uw.z - 1.0f};
        }

        Matrix part = AttackerPartMatrix(models, previous, current, i, alpha, time, orbit, targetXZ);
        for (int k = 0; k < muzzleCount; k++)
        {
            const Muzzle* mz = muzzles.ForUnit(u.type, k);
            if (mz == nullptr) continue;
            Vector3 spawn = Vector3Transform(mz->pos, part);
            Vector3 tip = Vector3Transform(Vector3Add(mz->pos, Vector3Scale(mz->dir, 0.4f)), part);
            DrawSphere(spawn, 0.06f, YELLOW);
            DrawLine3D(spawn, tip, ORANGE);
        }
    }
}
}
