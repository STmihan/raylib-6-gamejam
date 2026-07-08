#include "view/model_registry.h"

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

    water_ = LoadModelFromMesh(GenMeshPlane(800.0f, 800.0f, 1, 1));
    waterTexture_ = LoadTexture("assets/water.png");
    SetTextureWrap(waterTexture_, TEXTURE_WRAP_REPEAT);
    SetTextureFilter(waterTexture_, TEXTURE_FILTER_BILINEAR);
    water_.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = waterTexture_;

    loaded_ = true;
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
    UnloadTexture(waterTexture_);
    UnloadModel(water_);
    loaded_ = false;
}

void ModelRegistry::ApplyShader(Shader shader)
{
    Model* all[] = {
        &field_, &forest_, &concrete_, &swamp_, &swampEdge_, &swampCorner_, &red_,
        &wall_, &base_, &treeBush_, &treePine_, &treeRound_,
    };
    for (Model* model : all)
    {
        for (int i = 0; i < model->materialCount; i++)
        {
            model->materials[i].shader = shader;
        }
    }
}

void ModelRegistry::SetShadowMap(Texture2D shadowMap)
{
    Model* all[] = {
        &field_, &forest_, &concrete_, &swamp_, &swampEdge_, &swampCorner_, &red_,
        &wall_, &base_, &treeBush_, &treePine_, &treeRound_,
    };
    for (Model* model : all)
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
}
