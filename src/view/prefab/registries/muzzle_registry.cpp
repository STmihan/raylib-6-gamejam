#include "view/prefab/registries/muzzle_registry.h"

#include <cstdio>
#include <cstring>

namespace view
{
namespace
{
    constexpr const char* MuzzleFilePath = "assets/muzzles.txt";
}

void MuzzleRegistry::Load()
{
    muzzles_.clear();
    char* text = LoadFileText(MuzzleFilePath);
    if (text == nullptr) return;

    for (char* line = std::strtok(text, "\r\n"); line != nullptr; line = std::strtok(nullptr, "\r\n"))
    {
        if (line[0] == '#' || line[0] == '\0') continue;
        Muzzle m{};
        if (std::sscanf(line, "%31s %31s %f %f %f %f %f %f", m.name, m.parent, &m.pos.x, &m.pos.y, &m.pos.z,
                        &m.dir.x, &m.dir.y, &m.dir.z) == 8)
        {
            muzzles_.push_back(m);
        }
    }
    UnloadFileText(text);
}

const Muzzle* MuzzleRegistry::Get(const char* name) const
{
    for (const Muzzle& m : muzzles_)
    {
        if (std::strcmp(m.name, name) == 0) return &m;
    }
    return nullptr;
}

const Muzzle* MuzzleRegistry::ForUnit(data::UnitType type, int index) const
{
    char name[32];
    switch (type)
    {
    case data::UnitType::Infantry: std::snprintf(name, sizeof(name), "muzzle_infantry"); break;
    case data::UnitType::Rocketeer: std::snprintf(name, sizeof(name), "muzzle_rocket"); break;
    case data::UnitType::Tank: std::snprintf(name, sizeof(name), "muzzle_tank"); break;
    case data::UnitType::AA: std::snprintf(name, sizeof(name), "muzzle_pvo_%d", index); break;
    case data::UnitType::Plane: std::snprintf(name, sizeof(name), "muzzle_plane_%s", index == 0 ? "L" : "R"); break;
    default: return nullptr;
    }
    return Get(name);
}
}
