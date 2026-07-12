#include "debug/preview_scene.h"

#include <cmath>

#include "raymath.h"

#include "data/space/world_config.h"
#include "data/tile/tile.h"
#include "data/unit/unit.h"
#include "view/anim/hit_flash.h"
#include "view/prefab/registries/model_registry.h"
#include "view/prefab/vehicle_view.h"

namespace debug
{
using namespace view;

namespace
{
    const char* kNames[] = {
        "Tile: Field", "Tile: Forest", "Tile: Concrete", "Tile: Red",
        "Tile: Corner", "Tile: Empty", "Tile: Field2",
        "Wall", "Base",
        "Tree: Bush", "Tree: Pine", "Tree: Round",
        "Infantry", "Rocketeer", "Engineer", "Plane", "Tank", "PVO",
        "Ring: Blue", "Ring: Red",
    };
    constexpr int kCount = sizeof(kNames) / sizeof(kNames[0]);

    void DrawGrounded(const Model& model, float yawDeg, Color tint)
    {
        BoundingBox box = GetModelBoundingBox(model);
        DrawModelEx(model, Vector3{0.0f, -box.min.y, 0.0f}, Vector3{0.0f, 1.0f, 0.0f}, yawDeg,
                    Vector3{1.0f, 1.0f, 1.0f}, tint);
    }
}

void PreviewScene::Hit()
{
    flashTimer_ = HitFlashDuration;
}

void PreviewScene::UpdateFlash(float dt)
{
    if (flashTimer_ > 0.0f) flashTimer_ -= dt;
}

int PreviewScene::ItemCount() const
{
    return kCount;
}

const char* PreviewScene::ItemName(int index) const
{
    if (index < 0 || index >= kCount) return "";
    return kNames[index];
}

void PreviewScene::UpdateCamera()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        Vector2 delta = GetMouseDelta();
        yaw_ -= delta.x * 0.3f;
        pitch_ += delta.y * 0.3f;
        pitch_ = Clamp(pitch_, -85.0f, 85.0f);
    }
    distance_ -= GetMouseWheelMove() * 0.5f;
    distance_ = Clamp(distance_, 1.5f, 20.0f);
}

Camera3D PreviewScene::Camera() const
{
    float yr = yaw_ * DEG2RAD;
    float pr = pitch_ * DEG2RAD;
    Vector3 target = {0.0f, 0.5f, 0.0f};
    Vector3 position = {
        target.x + distance_ * std::cos(pr) * std::sin(yr),
        target.y + distance_ * std::sin(pr),
        target.z + distance_ * std::cos(pr) * std::cos(yr),
    };
    Camera3D camera = {};
    camera.position = position;
    camera.target = target;
    camera.up = Vector3{0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}

void PreviewScene::DrawGizmo() const
{
    float y = 0.04f;
    Vector3 origin = {0.0f, y, 0.0f};

    Vector3 frontShaft = {0.0f, y, -1.0f};
    Vector3 frontTip = {0.0f, y, -1.45f};
    DrawCylinderEx(origin, frontShaft, 0.03f, 0.03f, 10, GREEN);
    DrawCylinderEx(frontShaft, frontTip, 0.09f, 0.0f, 10, GREEN);

    Vector3 rightEnd = {0.7f, y, 0.0f};
    DrawCylinderEx(origin, rightEnd, 0.02f, 0.02f, 8, RED);

    if (selected_ >= 12 && selected_ <= 17)
    {
        data::UnitType unitTypes[6] = {
            data::UnitType::Infantry, data::UnitType::Rocketeer, data::UnitType::Engineer,
            data::UnitType::Plane, data::UnitType::Tank, data::UnitType::RL,
        };
        data::UnitType type = unitTypes[selected_ - 12];
        float radius = data::UnitStatsOf(type).footprint * data::RenderScale;
        if (radius > 0.001f)
        {
            Vector3 center = {0.0f, 0.02f, 0.0f};
            DrawCircle3D(center, radius, Vector3{1.0f, 0.0f, 0.0f}, 90.0f, YELLOW);
            DrawSphere(Vector3{radius, 0.02f, 0.0f}, 0.04f, ORANGE);
            DrawSphere(Vector3{-radius, 0.02f, 0.0f}, 0.04f, ORANGE);
            DrawSphere(Vector3{0.0f, 0.02f, radius}, 0.04f, ORANGE);
            DrawSphere(Vector3{0.0f, 0.02f, -radius}, 0.04f, ORANGE);
        }
    }
}

void PreviewScene::Draw(const ModelRegistry& models) const
{
    Color tint = HitFlashTint(flashTimer_ / HitFlashDuration);
    switch (selected_)
    {
    case 0: DrawGrounded(models.FloorFor(data::TileType::Field), 0.0f, tint); break;
    case 1: DrawGrounded(models.FloorFor(data::TileType::Forest), 0.0f, tint); break;
    case 2: DrawGrounded(models.FloorFor(data::TileType::ConcreteRoad), 0.0f, tint); break;
    case 3: DrawGrounded(models.FloorFor(data::TileType::RedBorder), 0.0f, tint); break;
    case 4: DrawGrounded(models.FloorFor(data::TileType::Corner), 0.0f, tint); break;
    case 5: DrawGrounded(models.FloorFor(data::TileType::Empty), 0.0f, tint); break;
    case 6: DrawGrounded(models.FloorFor(data::TileType::Field), 0.0f, tint); break;
    case 7: DrawGrounded(models.Wall(), 0.0f, tint); break;
    case 8:
    {
        BoundingBox box = GetModelBoundingBox(models.BaseSection());
        Vector3 pos = Vector3{0.0f, -box.min.y, 0.0f};
        DrawModelEx(models.BaseSection(), pos, Vector3{0.0f, 1.0f, 0.0f}, 0.0f, Vector3{1.0f, 1.0f, 1.0f}, tint);
        DrawModelEx(models.BaseTurret(), pos, Vector3{0.0f, 1.0f, 0.0f}, 0.0f, Vector3{1.0f, 1.0f, 1.0f}, tint);
    }
    break;
    case 9: DrawGrounded(models.Tree(0), 0.0f, tint); break;
    case 10: DrawGrounded(models.Tree(1), 0.0f, tint); break;
    case 11: DrawGrounded(models.Tree(2), 0.0f, tint); break;
    case 12: DrawGrounded(models.UnitBody(data::UnitType::Infantry), 0.0f, tint); break;
    case 13: DrawGrounded(models.UnitBody(data::UnitType::Rocketeer), 0.0f, tint); break;
    case 14: DrawGrounded(models.UnitBody(data::UnitType::Engineer), 0.0f, tint); break;
    case 15: DrawGrounded(models.UnitBody(data::UnitType::Plane), 0.0f, tint); break;
    case 16: DrawVehicle(models, data::UnitType::Tank, Vector3{0.0f, 0.0f, 0.0f}, 0.0f,
                         static_cast<float>(GetTime()), 0, true,
                         0.35f * std::sin(0.6f * static_cast<float>(GetTime())), tint); break;
    case 17: DrawVehicle(models, data::UnitType::RL, Vector3{0.0f, 0.0f, 0.0f}, 0.0f,
                         static_cast<float>(GetTime()), 0, true,
                         0.35f * std::sin(0.6f * static_cast<float>(GetTime())), tint); break;
    case 18: DrawGrounded(models.TeamRing(data::Team::Top), 0.0f, tint); break;
    case 19: DrawGrounded(models.TeamRing(data::Team::Bottom), 0.0f, tint); break;
    default: break;
    }
}
}
