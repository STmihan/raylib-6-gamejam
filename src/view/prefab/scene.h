#ifndef VIEW_PREFAB_SCENE_H
#define VIEW_PREFAB_SCENE_H

namespace logic { struct Map; }

namespace view
{
class ModelRegistry;

class Scene
{
public:
    explicit Scene(const ModelRegistry& models) : models_(&models) {}

    void DrawFloors(const logic::Map& map) const;
    void DrawStructures(const logic::Map& map) const;

private:
    const ModelRegistry* models_;
};
}

#endif
