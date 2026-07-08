#include "view/renderer.h"

#include "rlgl.h"

#include "data/hex.h"
#include "data/tile.h"
#include "data/vec.h"
#include "data/world_config.h"
#include "logic/map.h"
#include "view/interpolator.h"

namespace view {

namespace {

Vector3 LogicToWorld(data::Vec2 logic, float height) {
    return { logic.x * data::RenderScale, height, logic.y * data::RenderScale };
}

Color TileColor(data::TileType type) {
    switch (type) {
        case data::TileType::ConcreteRoad: return Color{ 150, 150, 150, 255 };
        case data::TileType::Base:         return Color{ 196, 54, 44, 255 };
        case data::TileType::Wall:         return Color{ 92, 80, 64, 255 };
        case data::TileType::Field:        return Color{ 98, 140, 56, 255 };
        case data::TileType::Forest:       return Color{ 40, 72, 40, 255 };
        case data::TileType::SwampEdge:    return Color{ 74, 84, 54, 255 };
        case data::TileType::SwampCenter:  return Color{ 52, 60, 40, 255 };
        case data::TileType::RedBorder:    return Color{ 176, 48, 40, 255 };
    }
    return Color{ 255, 0, 255, 255 };
}

void DrawHexTile(Vector3 world, float radius, float height, Color color) {
    rlPushMatrix();
        rlTranslatef(world.x, world.y, world.z);
        rlRotatef(30.0f, 0.0f, 1.0f, 0.0f);
        DrawCylinder(Vector3{ 0.0f, 0.0f, 0.0f }, radius, radius, height, 6, color);
    rlPopMatrix();
}

void DrawMap(const logic::Map &map) {
    float radius = data::HexRadius * 0.96f;
    for (int row = 0; row < logic::MapRows; row++) {
        for (int col = 0; col < logic::MapCols; col++) {
            Vector3 world = LogicToWorld(data::CellToLogic(col, row), 0.0f);
            DrawHexTile(world, radius, 0.12f, TileColor(map.At(col, row)));
        }
    }
}

void DrawEntities(const logic::GameState &previous, const logic::GameState &current, float alpha) {
    for (int i = 0; i < logic::MaxEntities; i++) {
        const logic::Entity &entity = current.entities[i];
        if (!entity.active) continue;

        data::Vec2 logic = InterpolatedPosition(previous.entities[i], entity, alpha);
        Vector3 world = LogicToWorld(logic, 0.4f);
        DrawCube(world, 0.5f, 0.5f, 0.5f, Color{ 60, 110, 190, 255 });
    }
}

}

void Renderer::Init() {
}

void Renderer::Shutdown() {
}

void Renderer::Draw(const logic::GameState &previous, const logic::GameState &current, float alpha, Camera3D camera, const logic::Map &map) {
    BeginDrawing();
        ClearBackground(Color{ 26, 26, 28, 255 });

        BeginMode3D(camera);
            DrawMap(map);
            DrawEntities(previous, current, alpha);
        EndMode3D();

        DrawFPS(10, 10);
    EndDrawing();
}

}
