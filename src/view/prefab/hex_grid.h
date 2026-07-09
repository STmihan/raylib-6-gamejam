#ifndef VIEW_PREFAB_HEX_GRID_H
#define VIEW_PREFAB_HEX_GRID_H

#include "raylib.h"

namespace logic { struct Map; }

namespace view
{
class HexGrid
{
public:
    void Load(const logic::Map& map);
    void Unload();
    void Draw();

    Vector3& ColorRef() { return color_; }
    float& ThicknessRef() { return thickness_; }
    float& OpacityRef() { return opacity_; }

private:
    void BuildMesh();

    const logic::Map* map_ = nullptr;
    Mesh mesh_{};
    Material material_{};
    Vector3 color_ = {0.000f, 0.000f, 0.000f};
    float opacity_ = 0.249f;
    float thickness_ = 0.055f;
    float builtThickness_ = 0.055f;
    bool loaded_ = false;
};
}

#endif
