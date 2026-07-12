#include "view/prefab/unit_view.h"

#include <cmath>

#include "raylib.h"
#include "raymath.h"

#include "view/anim/hit_flash.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/vehicle_view.h"
#include "view/space/interpolator.h"
#include "view/space/orientation.h"
#include "view/space/world_space.h"

namespace view
{
namespace
{
    constexpr float RingHeight = 0.02f;
    constexpr float BlobHeight = 0.012f;
    constexpr float MaxBlobDiameter = 1.5f;

    bool BodyTurnsToTarget(data::UnitType type)
    {
        return type == data::UnitType::Infantry || type == data::UnitType::Rocketeer;
    }

    float BlendYaw(float from, float to, float t)
    {
        float diff = std::fmod(to - from + 540.0f, 360.0f) - 180.0f;
        return from + diff * t;
    }

    Vector2 EntityXZ(const logic::GameState& previous, const logic::GameState& current, int index, float alpha)
    {
        data::Vec2 logic = InterpolatedPosition(previous.entities[index], current.entities[index], alpha);
        Vector3 world = LogicToWorld(logic, 0.0f);
        return Vector2{world.x, world.z};
    }

    void DrawMeshTinted(const Model& model, int meshIndex, Matrix transform, Color tint)
    {
        Material& mat = model.materials[model.meshMaterial[meshIndex]];
        Color original = mat.maps[MATERIAL_MAP_DIFFUSE].color;
        mat.maps[MATERIAL_MAP_DIFFUSE].color = ColorTint(original, tint);
        DrawMesh(model.meshes[meshIndex], mat, transform);
        mat.maps[MATERIAL_MAP_DIFFUSE].color = original;
    }

    void DrawBody(const Model& model, Vector3 position, float yaw, Color tint)
    {
        DrawModelEx(model, position, Vector3{0.0f, 1.0f, 0.0f}, yaw, Vector3{1.0f, 1.0f, 1.0f}, tint);
    }

    void DrawPlane(const Model& model, Vector3 position, float yawDeg, float rollDeg, Color tint)
    {
        Matrix m = MatrixRotateZ(rollDeg * DEG2RAD);
        m = MatrixMultiply(m, MatrixRotateY(yawDeg * DEG2RAD));
        m = MatrixMultiply(m, MatrixTranslate(position.x, position.y, position.z));
        for (int i = 0; i < model.meshCount; i++)
        {
            DrawMeshTinted(model, i, m, tint);
        }
    }
}

void UnitView::UpdateFlash(const logic::GameState& current, float dt)
{
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& entity = current.entities[i];
        if (entity.active && entity.kind == logic::EntityKind::Unit)
        {
            if (entity.hp < lastHp_[i]) flashTimer_[i] = HitFlashDuration;
            lastHp_[i] = entity.hp;
        }
        else
        {
            lastHp_[i] = 0;
            flashTimer_[i] = 0.0f;
        }
        if (flashTimer_[i] > 0.0f) flashTimer_[i] -= dt;

        bool combat = entity.active && entity.type == data::UnitType::Plane && entity.targetSlot >= 0;
        float rate = dt / 0.4f;
        combatBlend_[i] += combat ? rate : -rate;
        if (combatBlend_[i] < 0.0f) combatBlend_[i] = 0.0f;
        if (combatBlend_[i] > 1.0f) combatBlend_[i] = 1.0f;
    }
}

void UnitView::TriggerFlashAll()
{
    for (int i = 0; i < data::MaxEntities; i++) flashTimer_[i] = HitFlashDuration;
}

void UnitView::Draw(const ModelRegistry& models, const logic::GameState& previous,
                    const logic::GameState& current, float alpha, bool includeDecals, bool drawRings,
                    const PlaneOrbitParams& orbit, float time) const
{
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& entity = current.entities[i];
        if (!entity.active || entity.kind != logic::EntityKind::Unit) continue;

        data::Vec2 logic = InterpolatedPosition(previous.entities[i], entity, alpha);
        Vector3 ground = LogicToWorld(logic, 0.0f);
        bool attacking = entity.targetSlot >= 0;
        Color tint = HitFlashTint(flashTimer_[i] / HitFlashDuration);

        const logic::Entity& prevEntity = previous.entities[i];
        Vector3 prevGround = LogicToWorld(prevEntity.position, 0.0f);
        float moveYaw = prevEntity.active
            ? YawTowards(Vector2{prevGround.x, prevGround.z}, Vector2{ground.x, ground.z}, TeamYaw(entity.team))
            : TeamYaw(entity.team);

        if (includeDecals)
        {
            if (entity.type != data::UnitType::Plane)
            {
                float diameter = data::UnitStatsOf(entity.type).footprint * data::RenderScale * blobRadius_ * 2.0f;
                if (diameter > MaxBlobDiameter) diameter = MaxBlobDiameter;
                Vector3 blobPos = LogicToWorld(logic, BlobHeight);
                auto blobAlpha = static_cast<unsigned char>(Clamp(blobOpacity_, 0.0f, 1.0f) * 255.0f);
                DrawModelEx(models.BlobShadow(), blobPos, Vector3{0.0f, 1.0f, 0.0f}, 0.0f,
                            Vector3{diameter, 1.0f, diameter}, Color{0, 0, 0, blobAlpha});
            }
            if (drawRings)
            {
                Vector3 ringPos = LogicToWorld(logic, RingHeight);
                DrawModelYaw(models.TeamRing(entity.team), ringPos, 0.0f, WHITE);
            }
        }

        if (entity.type == data::UnitType::Plane)
        {
            float yaw;
            float roll;
            Vector3 orbitPos = PlaneVisualPos(previous, current, i, alpha, time, orbit, yaw, roll);
            float blend = combatBlend_[i];
            Vector3 hover = {ground.x, orbit.altitude, ground.z};
            Vector3 pos = Vector3Lerp(hover, orbitPos, blend);
            float drawYaw = BlendYaw(moveYaw, yaw, blend);
            DrawPlane(models.UnitBody(data::UnitType::Plane), pos, drawYaw, roll * blend, tint);
            continue;
        }

        if (data::UnitStatsOf(entity.type).isVehicle)
        {
            float bodyYaw = moveYaw;
            float turretYaw;
            if (attacking)
            {
                float aim = YawTowards(Vector2{ground.x, ground.z},
                                       EntityXZ(previous, current, entity.targetSlot, alpha), bodyYaw);
                turretYaw = (aim - bodyYaw) * DEG2RAD;
            }
            else
            {
                turretYaw = 0.35f * std::sin(0.6f * time + static_cast<float>(entity.id));
            }
            DrawVehicle(models, entity.type, ground, bodyYaw, time, static_cast<int>(entity.id), includeDecals,
                        turretYaw, tint);
            continue;
        }

        float yaw = moveYaw;
        if (attacking && BodyTurnsToTarget(entity.type))
        {
            yaw = YawTowards(Vector2{ground.x, ground.z},
                             EntityXZ(previous, current, entity.targetSlot, alpha), yaw);
        }
        Vector3 bodyPos = LogicToWorld(logic, models.UnitGroundOffset(entity.type));
        DrawBody(models.UnitBody(entity.type), bodyPos, yaw, tint);
    }
}

void UnitView::DrawPreview(const ModelRegistry& models, data::UnitType type, data::Vec2 logic, data::Team team,
                           const PlaneOrbitParams& orbit, float time, Color tint) const
{
    Vector3 ground = LogicToWorld(logic, 0.0f);
    float yaw = TeamYaw(team);

    if (type == data::UnitType::Plane)
    {
        Vector3 pos = {ground.x, orbit.altitude, ground.z};
        DrawPlane(models.UnitBody(data::UnitType::Plane), pos, yaw, 0.0f, tint);
        return;
    }
    if (data::UnitStatsOf(type).isVehicle)
    {
        float turretYaw = 0.35f * std::sin(0.6f * time);
        DrawVehicle(models, type, ground, yaw, time, 0, true, turretYaw, tint);
        return;
    }
    Vector3 bodyPos = LogicToWorld(logic, models.UnitGroundOffset(type));
    DrawBody(models.UnitBody(type), bodyPos, yaw, tint);
}

void UnitView::DrawHighlight(const ModelRegistry& models, const logic::GameState& previous,
                             const logic::GameState& current, float alpha, const PlaneOrbitParams& orbit,
                             float time, int index, Color tint) const
{
    const logic::Entity& entity = current.entities[index];
    if (!entity.active) return;

    data::Vec2 logic = InterpolatedPosition(previous.entities[index], entity, alpha);
    Vector3 ground = LogicToWorld(logic, 0.0f);
    bool attacking = entity.targetSlot >= 0;

    const logic::Entity& prevEntity = previous.entities[index];
    Vector3 prevGround = LogicToWorld(prevEntity.position, 0.0f);
    float moveYaw = prevEntity.active
        ? YawTowards(Vector2{prevGround.x, prevGround.z}, Vector2{ground.x, ground.z}, TeamYaw(entity.team))
        : TeamYaw(entity.team);

    if (entity.type == data::UnitType::Plane)
    {
        float yaw;
        float roll;
        Vector3 orbitPos = PlaneVisualPos(previous, current, index, alpha, time, orbit, yaw, roll);
        float blend = combatBlend_[index];
        Vector3 hover = {ground.x, orbit.altitude, ground.z};
        Vector3 pos = Vector3Lerp(hover, orbitPos, blend);
        float drawYaw = attacking ? yaw : moveYaw;
        DrawPlane(models.UnitBody(data::UnitType::Plane), pos, drawYaw, roll * blend, tint);
        return;
    }

    if (data::UnitStatsOf(entity.type).isVehicle)
    {
        DrawVehicle(models, entity.type, ground, moveYaw, time, static_cast<int>(entity.id), false, 0.0f, tint);
        return;
    }

    Vector3 bodyPos = LogicToWorld(logic, models.UnitGroundOffset(entity.type));
    DrawBody(models.UnitBody(entity.type), bodyPos, moveYaw, tint);
}
}
