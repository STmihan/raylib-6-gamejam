#ifndef VIEW_MODEL_REGISTRY_H
#define VIEW_MODEL_REGISTRY_H

#include "raylib.h"

#include "data/tile/tile.h"

namespace view
{
class ModelRegistry
{
public:
    void Load();
    void Unload();
    void ApplyShader(Shader shader);
    void SetShadowMap(Texture2D shadowMap);
    void SetWaterShader(Shader shader);

    const Model& FloorFor(data::TileType type) const;
    const Model& Wall() const;
    const Model& BaseSection() const;
    const Model& Tree(int index) const;
    const Model& Water() const;

private:
    Model field_{};
    Model forest_{};
    Model concrete_{};
    Model swamp_{};
    Model swampEdge_{};
    Model swampCorner_{};
    Model red_{};
    Model wall_{};
    Model base_{};
    Model treeBush_{};
    Model treePine_{};
    Model treeRound_{};
    Model water_{};
    Texture2D waterTexture_{};
    bool loaded_ = false;
};
}

#endif
