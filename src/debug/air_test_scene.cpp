#include "debug/air_test_scene.h"

#include <cmath>

#include "raymath.h"

#include "data/space/hex.h"
#include "data/unit/unit.h"

namespace debug
{
namespace
{
    constexpr float TwoPi = 6.2831853f;
    constexpr float SpreadLogic = 90.0f;
}

void AirTestScene::UpdateCamera()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        Vector2 delta = GetMouseDelta();
        yaw_ -= delta.x * 0.3f;
        pitch_ += delta.y * 0.3f;
        pitch_ = Clamp(pitch_, -85.0f, 85.0f);
    }
    distance_ -= GetMouseWheelMove() * 0.5f;
    distance_ = Clamp(distance_, 2.0f, 30.0f);
}

Camera3D AirTestScene::Camera() const
{
    float yr = yaw_ * DEG2RAD;
    float pr = pitch_ * DEG2RAD;
    Vector3 target = {0.0f, 1.0f, 0.0f};
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

void AirTestScene::Rebuild()
{
    state_ = logic::GameState{};
    state_.winner = -1;
    for (int i = 0; i < data::MaxEntities; i++) state_.entities[i].active = false;

    int planes = planeCount_ < 1 ? 1 : planeCount_;
    int count = 0;

    auto add = [&](data::UnitType type, data::Team team, data::Vec2 pos, int targetSlot) {
        logic::Entity& e = state_.entities[count];
        e.active = true;
        e.id = static_cast<std::uint32_t>(count);
        e.kind = logic::EntityKind::Unit;
        e.type = type;
        e.team = team;
        e.position = pos;
        data::Offset cell = data::CellFromLogic(pos);
        e.col = cell.col;
        e.row = cell.row;
        e.hp = data::UnitStatsOf(type).hp;
        e.targetSlot = targetSlot;
        e.attackCooldown = 0.0f;
        count++;
    };

    if (mode_ == 0)
    {
        for (int k = 0; k < planes; k++)
        {
            float a = TwoPi * static_cast<float>(k) / static_cast<float>(planes);
            data::Vec2 pos = {std::cos(a) * SpreadLogic, std::sin(a) * SpreadLogic};
            data::Team team = k % 2 == 0 ? data::Team::Top : data::Team::Bottom;
            add(data::UnitType::Plane, team, pos, (k + 1) % planes);
        }
    }
    else
    {
        add(data::UnitType::Tank, data::Team::Top, {0.0f, 0.0f}, -1);
        for (int k = 0; k < planes; k++)
        {
            float a = TwoPi * static_cast<float>(k) / static_cast<float>(planes);
            data::Vec2 pos = {std::cos(a) * SpreadLogic, std::sin(a) * SpreadLogic};
            add(data::UnitType::Plane, data::Team::Bottom, pos, 0);
        }
    }

    state_.entityCount = count;
    builtCount_ = planeCount_;
    builtMode_ = mode_;
}

const logic::GameState& AirTestScene::State()
{
    if (planeCount_ != builtCount_ || mode_ != builtMode_) Rebuild();
    return state_;
}
}
