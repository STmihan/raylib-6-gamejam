#ifndef VIEW_MODEL_REGISTRY_H
#define VIEW_MODEL_REGISTRY_H

#include <array>
#include <vector>

#include "raylib.h"

#include "data/tile/tile.h"
#include "data/unit/unit.h"

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
    const Model& UnitBody(data::UnitType type) const;
    const Model& TeamRing(data::Team team) const;
    const Model& Shell() const;
    const Model& BlobShadow() const;

    struct VehicleModels
    {
        const Model* hull;
        const Model* turret;
        const Model* wheels;
        const Model* band;
        const Model* link;
    };
    VehicleModels VehicleFor(data::UnitType type) const;

    float UnitGroundOffset(data::UnitType type) const;
    float VehicleLowestY(data::UnitType type) const;
    const std::vector<Vector3>& WheelCenters(data::UnitType type) const;

private:
    static constexpr int ShadedCount = 26;
    std::array<Model*, ShadedCount> ShadedModels();
    void CacheBounds();

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
    Model soldierInfantry_{};
    Model soldierRocket_{};
    Model soldierEngineer_{};
    Model plane_{};
    Model tankHull_{};
    Model tankTurret_{};
    Model tankWheels_{};
    Model tankBand_{};
    Model pvoHull_{};
    Model pvoLauncher_{};
    Model pvoWheels_{};
    Model pvoBand_{};
    Model trackLink_{};
    Model shell_{};
    Model ringBlue_{};
    Model ringRed_{};
    Model blobShadow_{};
    Texture2D blobTexture_{};
    Model water_{};
    Texture2D waterTexture_{};

    std::array<float, data::UnitTypeCount> unitGroundOffset_{};
    float tankLowestY_ = 0.0f;
    float pvoLowestY_ = 0.0f;
    std::vector<Vector3> tankWheelCenters_;
    std::vector<Vector3> pvoWheelCenters_;

    bool loaded_ = false;
};
}

#endif
