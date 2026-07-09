#ifndef VIEW_PREFAB_VEHICLE_VIEW_H
#define VIEW_PREFAB_VEHICLE_VIEW_H

#include "raylib.h"

#include "data/unit/unit.h"

namespace view
{
class ModelRegistry;

void DrawVehicle(const ModelRegistry& models, data::UnitType type, Vector3 position, float yawDeg,
                 float time, int id, bool fullDetail, float turretYaw, Color tint);

Matrix VehicleWorldMatrix(const ModelRegistry& models, data::UnitType type, Vector3 position, float yawDeg);
Matrix VehicleTurretMatrix(const ModelRegistry& models, data::UnitType type, Vector3 position, float yawDeg,
                           float turretYaw);
}

#endif
