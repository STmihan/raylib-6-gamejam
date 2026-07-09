#ifndef VIEW_PREFAB_REGISTRIES_MUZZLE_REGISTRY_H
#define VIEW_PREFAB_REGISTRIES_MUZZLE_REGISTRY_H

#include <vector>

#include "raylib.h"

#include "data/unit/unit.h"

namespace view
{
struct Muzzle
{
    char name[32];
    char parent[32];
    Vector3 pos;
    Vector3 dir;
};

class MuzzleRegistry
{
public:
    void Load();
    const Muzzle* Get(const char* name) const;
    const Muzzle* ForUnit(data::UnitType type, int index) const;

private:
    std::vector<Muzzle> muzzles_;
};
}

#endif
