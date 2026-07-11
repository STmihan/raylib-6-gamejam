#ifndef VIEW_PREFAB_SCENE_H
#define VIEW_PREFAB_SCENE_H

#include "raylib.h"

#include "data/unit/unit.h"

namespace logic { struct Map; }

namespace view
{
class ModelRegistry;

class Scene
{
public:
    explicit Scene(const ModelRegistry& models) : models_(&models) {}

    void Draw(const logic::Map& map, bool includeFloors = true, const bool* occluded = nullptr,
              const bool* wallAlive = nullptr) const;
    void DrawGhostStructures(const logic::Map& map, const bool* occluded, const bool* wallAlive,
                             unsigned char alpha) const;
    void DrawBaseHighlight(const logic::Map& map, data::Team team, Color tint) const;
    void SetBaseAim(data::Team team, Vector3 aim, bool hasTarget);

private:
    void DrawFloors(const logic::Map& map) const;
    void DrawStructures(const logic::Map& map, const bool* occluded, const bool* wallAlive) const;

    const ModelRegistry* models_;
    Vector3 baseAim_[2]{};
    bool baseHasTarget_[2]{};
};
}

#endif
