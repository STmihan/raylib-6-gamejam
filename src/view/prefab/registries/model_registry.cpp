#include "view/prefab/registries/model_registry.h"

#include <cmath>
#include <initializer_list>

#include "data/render/render_params.h"

namespace view
{
void ModelRegistry::Load()
{
    field_ = LoadModel("assets/models/tile_field.glb");
    forest_ = LoadModel("assets/models/tile_forest.glb");
    concrete_ = LoadModel("assets/models/tile_concrete.glb");
    swamp_ = LoadModel("assets/models/tile_swamp.glb");
    swampEdge_ = LoadModel("assets/models/tile_swamp_edge.glb");
    swampCorner_ = LoadModel("assets/models/tile_swamp_corner.glb");
    red_ = LoadModel("assets/models/tile_red.glb");
    wall_ = LoadModel("assets/models/wall.glb");
    base_ = LoadModel("assets/models/base_1x2.glb");
    treeBush_ = LoadModel("assets/models/tree_bush.glb");
    treePine_ = LoadModel("assets/models/tree_pine.glb");
    treeRound_ = LoadModel("assets/models/tree_round.glb");
    soldierInfantry_ = LoadModel("assets/models/soldier_infantry.glb");
    soldierRocket_ = LoadModel("assets/models/soldier_rocket.glb");
    soldierEngineer_ = LoadModel("assets/models/soldier_engineer.glb");
    plane_ = LoadModel("assets/models/plane.glb");
    tankHull_ = LoadModel("assets/models/tank_hull.glb");
    tankTurret_ = LoadModel("assets/models/tank_turret.glb");
    tankWheels_ = LoadModel("assets/models/tank_wheels.glb");
    tankBand_ = LoadModel("assets/models/tank_band.glb");
    pvoHull_ = LoadModel("assets/models/pvo_hull.glb");
    pvoLauncher_ = LoadModel("assets/models/pvo_launcher.glb");
    pvoWheels_ = LoadModel("assets/models/pvo_wheels.glb");
    pvoBand_ = LoadModel("assets/models/pvo_band.glb");
    trackLink_ = LoadModel("assets/models/track_link.glb");
    shell_ = LoadModel("assets/models/shell.glb");
    ringBlue_ = LoadModel("assets/models/circle_blue.glb");
    ringRed_ = LoadModel("assets/models/circle_red.glb");

    Image blobImage = GenImageGradientRadial(64, 64, 0.35f, Color{0, 0, 0, 255}, Color{0, 0, 0, 0});
    blobTexture_ = LoadTextureFromImage(blobImage);
    UnloadImage(blobImage);
    SetTextureFilter(blobTexture_, TEXTURE_FILTER_BILINEAR);
    blobShadow_ = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1));
    blobShadow_.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = blobTexture_;

    water_ = LoadModelFromMesh(GenMeshPlane(data::Render.waterPlaneSize, data::Render.waterPlaneSize, 1, 1));
    waterTexture_ = LoadTexture("assets/water.png");
    SetTextureWrap(waterTexture_, TEXTURE_WRAP_REPEAT);
    SetTextureFilter(waterTexture_, TEXTURE_FILTER_BILINEAR);
    water_.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = waterTexture_;

    CacheBounds();
    loaded_ = true;
}

void ModelRegistry::CacheBounds()
{
    auto lowestY = [](std::initializer_list<const Model*> models)
    {
        float value = 1e9f;
        for (const Model* model : models)
        {
            value = fminf(value, GetModelBoundingBox(*model).min.y);
        }
        return value;
    };
    auto meshCenters = [](const Model& model)
    {
        std::vector<Vector3> centers;
        for (int i = 0; i < model.meshCount; i++)
        {
            BoundingBox box = GetMeshBoundingBox(model.meshes[i]);
            centers.push_back(Vector3{(box.min.x + box.max.x) * 0.5f, (box.min.y + box.max.y) * 0.5f,
                                      (box.min.z + box.max.z) * 0.5f});
        }
        return centers;
    };

    tankLowestY_ = lowestY({&tankHull_, &tankBand_, &tankWheels_});
    pvoLowestY_ = lowestY({&pvoHull_, &pvoBand_, &pvoWheels_});
    tankWheelCenters_ = meshCenters(tankWheels_);
    pvoWheelCenters_ = meshCenters(pvoWheels_);

    for (int i = 0; i < data::UnitTypeCount; i++)
    {
        const Model& body = UnitBody(static_cast<data::UnitType>(i));
        unitGroundOffset_[static_cast<std::size_t>(i)] = -GetModelBoundingBox(body).min.y;
    }
}

void ModelRegistry::Unload()
{
    if (!loaded_) return;
    UnloadModel(field_);
    UnloadModel(forest_);
    UnloadModel(concrete_);
    UnloadModel(swamp_);
    UnloadModel(swampEdge_);
    UnloadModel(swampCorner_);
    UnloadModel(red_);
    UnloadModel(wall_);
    UnloadModel(base_);
    UnloadModel(treeBush_);
    UnloadModel(treePine_);
    UnloadModel(treeRound_);
    UnloadModel(soldierInfantry_);
    UnloadModel(soldierRocket_);
    UnloadModel(soldierEngineer_);
    UnloadModel(plane_);
    UnloadModel(tankHull_);
    UnloadModel(tankTurret_);
    UnloadModel(tankWheels_);
    UnloadModel(tankBand_);
    UnloadModel(pvoHull_);
    UnloadModel(pvoLauncher_);
    UnloadModel(pvoWheels_);
    UnloadModel(pvoBand_);
    UnloadModel(trackLink_);
    UnloadModel(shell_);
    UnloadModel(ringBlue_);
    UnloadModel(ringRed_);
    UnloadModel(blobShadow_);
    UnloadTexture(blobTexture_);
    UnloadTexture(waterTexture_);
    UnloadModel(water_);
    loaded_ = false;
}

std::array<Model*, ModelRegistry::ShadedCount> ModelRegistry::ShadedModels()
{
    return {
        &field_, &forest_, &concrete_, &swamp_, &swampEdge_, &swampCorner_, &red_,
        &wall_, &base_, &treeBush_, &treePine_, &treeRound_,
        &soldierInfantry_, &soldierRocket_, &soldierEngineer_, &plane_,
        &tankHull_, &tankTurret_, &tankWheels_, &tankBand_,
        &pvoHull_, &pvoLauncher_, &pvoWheels_, &pvoBand_, &trackLink_,
        &shell_,
    };
}

void ModelRegistry::ApplyShader(Shader shader)
{
    for (Model* model : ShadedModels())
    {
        for (int i = 0; i < model->materialCount; i++)
        {
            model->materials[i].shader = shader;
        }
    }
}

void ModelRegistry::SetShadowMap(Texture2D shadowMap)
{
    for (Model* model : ShadedModels())
    {
        for (int i = 0; i < model->materialCount; i++)
        {
            model->materials[i].maps[MATERIAL_MAP_NORMAL].texture = shadowMap;
        }
    }
}

void ModelRegistry::SetWaterShader(Shader shader)
{
    for (int i = 0; i < water_.materialCount; i++)
    {
        water_.materials[i].shader = shader;
    }
}

const Model& ModelRegistry::FloorFor(data::TileType type) const
{
    switch (type)
    {
    case data::TileType::Field: return field_;
    case data::TileType::Forest: return forest_;
    case data::TileType::ConcreteRoad: return concrete_;
    case data::TileType::Base: return concrete_;
    case data::TileType::Wall: return concrete_;
    case data::TileType::SwampCenter: return swamp_;
    case data::TileType::SwampEdge: return swampEdge_;
    case data::TileType::SwampCorner: return swampCorner_;
    case data::TileType::RedBorder: return red_;
    }
    return field_;
}

const Model& ModelRegistry::Wall() const
{
    return wall_;
}

const Model& ModelRegistry::BaseSection() const
{
    return base_;
}

const Model& ModelRegistry::Tree(int index) const
{
    switch (index % 3)
    {
    case 0: return treeBush_;
    case 1: return treePine_;
    default: return treeRound_;
    }
}

const Model& ModelRegistry::Water() const
{
    return water_;
}

const Model& ModelRegistry::UnitBody(data::UnitType type) const
{
    switch (type)
    {
    case data::UnitType::Infantry: return soldierInfantry_;
    case data::UnitType::Rocketeer: return soldierRocket_;
    case data::UnitType::Engineer: return soldierEngineer_;
    case data::UnitType::AA: return pvoHull_;
    case data::UnitType::Tank: return tankHull_;
    case data::UnitType::Plane: return plane_;
    }
    return soldierInfantry_;
}

const Model& ModelRegistry::TeamRing(data::Team team) const
{
    return team == data::Team::Top ? ringBlue_ : ringRed_;
}

const Model& ModelRegistry::Shell() const
{
    return shell_;
}

const Model& ModelRegistry::BlobShadow() const
{
    return blobShadow_;
}

ModelRegistry::VehicleModels ModelRegistry::VehicleFor(data::UnitType type) const
{
    if (type == data::UnitType::AA)
    {
        return {&pvoHull_, &pvoLauncher_, &pvoWheels_, &pvoBand_, &trackLink_};
    }
    return {&tankHull_, &tankTurret_, &tankWheels_, &tankBand_, &trackLink_};
}

float ModelRegistry::UnitGroundOffset(data::UnitType type) const
{
    return unitGroundOffset_[static_cast<std::size_t>(type)];
}

float ModelRegistry::VehicleLowestY(data::UnitType type) const
{
    return type == data::UnitType::AA ? pvoLowestY_ : tankLowestY_;
}

const std::vector<Vector3>& ModelRegistry::WheelCenters(data::UnitType type) const
{
    return type == data::UnitType::AA ? pvoWheelCenters_ : tankWheelCenters_;
}
}
