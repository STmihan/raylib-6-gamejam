#include "view/renderer.h"

#include "data/vec.h"
#include "data/world_config.h"
#include "view/interpolator.h"

namespace view {

namespace {

Vector3 LogicToWorld(data::Vec2 logic, float height) {
    return { logic.x * data::RenderScale, height, logic.y * data::RenderScale };
}

}

void Renderer::Init() {
}

void Renderer::Shutdown() {
}

void Renderer::Draw(const logic::GameState &previous, const logic::GameState &current, float alpha, Camera3D camera) {
    BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
            DrawGrid(32, 1.0f);
            for (int i = 0; i < logic::MaxEntities; i++) {
                const logic::Entity &entity = current.entities[i];
                if (!entity.active) continue;

                data::Vec2 logic = InterpolatedPosition(previous.entities[i], entity, alpha);
                Vector3 world = LogicToWorld(logic, 0.3f);
                DrawCube(world, 0.6f, 0.6f, 0.6f, DARKBLUE);
            }
        EndMode3D();

        DrawFPS(10, 10);
    EndDrawing();
}

}
