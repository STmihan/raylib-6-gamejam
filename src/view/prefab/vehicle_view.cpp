#include "view/prefab/vehicle_view.h"

#include <cmath>
#include <vector>

#include "raymath.h"

#include "data/space/world_config.h"
#include "view/prefab/registries/model_registry.h"

namespace view
{
namespace
{
    constexpr float Pi = 3.14159265f;
    constexpr float LinkLen = 0.13f;
    constexpr float TurretMountZ = 0.05f;

    struct TrackSpec
    {
        float trackX;
        float frontU;
        float rearU;
        float axleV;
        float radius;
        float wheelR;
    };

    TrackSpec SpecFor(data::UnitType type)
    {
        if (type == data::UnitType::RL)
        {
            return {0.44f, 0.46f, -0.46f, 0.10f, 0.14f, 0.11f};
        }
        return {0.44f, 0.52f, -0.52f, 0.10f, 0.14f, 0.11f};
    }

    void LoopPoint(float s, const TrackSpec& t, float& u, float& v, float& phi)
    {
        float run = t.frontU - t.rearU;
        float length = 2.0f * run + 2.0f * Pi * t.radius;
        s = std::fmod(std::fmod(s, length) + length, length);

        if (s < run) { u = t.rearU + s; v = t.axleV - t.radius; phi = -Pi / 2.0f; return; }
        s -= run;
        if (s < Pi * t.radius) { float a = -Pi / 2.0f + s / t.radius; u = t.frontU + t.radius * std::cos(a); v = t.axleV + t.radius * std::sin(a); phi = a; return; }
        s -= Pi * t.radius;
        if (s < run) { u = t.frontU - s; v = t.axleV + t.radius; phi = Pi / 2.0f; return; }
        s -= run;
        float b = Pi / 2.0f + s / t.radius;
        u = t.rearU + t.radius * std::cos(b);
        v = t.axleV + t.radius * std::sin(b);
        phi = b;
    }

    void DrawMeshTinted(const Model& model, int meshIndex, Matrix transform, Color tint)
    {
        Material& mat = model.materials[model.meshMaterial[meshIndex]];
        Color original = mat.maps[MATERIAL_MAP_DIFFUSE].color;
        mat.maps[MATERIAL_MAP_DIFFUSE].color = ColorTint(original, tint);
        DrawMesh(model.meshes[meshIndex], mat, transform);
        mat.maps[MATERIAL_MAP_DIFFUSE].color = original;
    }

    void DrawModelMeshes(const Model& model, Matrix transform, Color tint)
    {
        for (int i = 0; i < model.meshCount; i++)
        {
            DrawMeshTinted(model, i, transform, tint);
        }
    }
}

Matrix VehicleWorldMatrix(const ModelRegistry& models, data::UnitType type, Vector3 position, float yawDeg)
{
    TrackSpec spec = SpecFor(type);
    float lowest = fminf(models.VehicleLowestY(type), spec.axleV - spec.radius - 0.03f);
    float groundY = position.y - lowest;
    return MatrixMultiply(MatrixRotateY(yawDeg * DEG2RAD), MatrixTranslate(position.x, groundY, position.z));
}

Matrix VehicleTurretMatrix(const ModelRegistry& models, data::UnitType type, Vector3 position, float yawDeg,
                           float turretYaw)
{
    Matrix mt = MatrixRotateY(turretYaw);
    mt = MatrixMultiply(mt, MatrixTranslate(0.0f, 0.0f, TurretMountZ));
    mt = MatrixMultiply(mt, VehicleWorldMatrix(models, type, position, yawDeg));
    return mt;
}

void DrawVehicle(const ModelRegistry& models, data::UnitType type, Vector3 position, float yawDeg,
                 float time, int id, bool fullDetail, float turretYaw, Color tint)
{
    ModelRegistry::VehicleModels parts = models.VehicleFor(type);
    TrackSpec spec = SpecFor(type);

    Matrix vehicleWorld = VehicleWorldMatrix(models, type, position, yawDeg);

    float speed = data::UnitStatsOf(type).moveSpeed * data::RenderScale;
    float phase = time + static_cast<float>(id) * 0.37f;
    float trackOffset = -speed * phase;
    float wheelAngle = (speed / spec.wheelR) * phase;

    DrawModelMeshes(*parts.hull, vehicleWorld, tint);
    DrawModelMeshes(*parts.band, vehicleWorld, tint);

    if (fullDetail)
    {
        const std::vector<Vector3>& centers = models.WheelCenters(type);
        for (int m = 0; m < parts.wheels->meshCount; m++)
        {
            Vector3 c = centers[static_cast<std::size_t>(m)];
            Matrix mw = MatrixTranslate(-c.x, -c.y, -c.z);
            mw = MatrixMultiply(mw, MatrixRotateX(wheelAngle));
            mw = MatrixMultiply(mw, MatrixTranslate(c.x, c.y, c.z));
            mw = MatrixMultiply(mw, vehicleWorld);
            DrawMeshTinted(*parts.wheels, m, mw, tint);
        }

        float run = spec.frontU - spec.rearU;
        float loopLen = 2.0f * run + 2.0f * Pi * spec.radius;
        int linkCount = static_cast<int>(std::round(loopLen / LinkLen));
        for (int side = 0; side < 2; side++)
        {
            float x = side == 0 ? spec.trackX : -spec.trackX;
            for (int i = 0; i < linkCount; i++)
            {
                float s = static_cast<float>(i) * loopLen / static_cast<float>(linkCount) + trackOffset;
                float u, v, phi;
                LoopPoint(s, spec, u, v, phi);
                Matrix ml = MatrixRotateX(phi - Pi / 2.0f);
                ml = MatrixMultiply(ml, MatrixTranslate(x, v, -u));
                ml = MatrixMultiply(ml, vehicleWorld);
                DrawMeshTinted(*parts.link, 0, ml, tint);
            }
        }
    }

    Matrix mt = VehicleTurretMatrix(models, type, position, yawDeg, turretYaw);
    DrawModelMeshes(*parts.turret, mt, tint);
}
}
