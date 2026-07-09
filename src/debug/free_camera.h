#ifndef DEBUG_FREE_CAMERA_H
#define DEBUG_FREE_CAMERA_H

#include <cmath>

#include "raylib.h"
#include "raymath.h"

namespace debug
{
class FreeCamera
{
public:
    void Sync(Camera3D from)
    {
        position_ = from.position;
        Vector3 dir = Vector3Normalize(Vector3Subtract(from.target, from.position));
        pitch_ = std::asin(Clamp(dir.y, -1.0f, 1.0f));
        yaw_ = std::atan2(dir.x, dir.z);
    }

    void Update(float dt)
    {
        Vector2 md = GetMouseDelta();
        yaw_ -= md.x * 0.0025f;
        pitch_ -= md.y * 0.0025f;
        pitch_ = Clamp(pitch_, -1.5f, 1.5f);

        Vector3 forward = Forward();
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, Vector3{0.0f, 1.0f, 0.0f}));
        float speed = (IsKeyDown(KEY_LEFT_SHIFT) ? 14.0f : 4.5f) * dt;

        if (IsKeyDown(KEY_W)) position_ = Vector3Add(position_, Vector3Scale(forward, speed));
        if (IsKeyDown(KEY_S)) position_ = Vector3Subtract(position_, Vector3Scale(forward, speed));
        if (IsKeyDown(KEY_D)) position_ = Vector3Add(position_, Vector3Scale(right, speed));
        if (IsKeyDown(KEY_A)) position_ = Vector3Subtract(position_, Vector3Scale(right, speed));
        if (IsKeyDown(KEY_E) || IsKeyDown(KEY_SPACE)) position_.y += speed;
        if (IsKeyDown(KEY_Q) || IsKeyDown(KEY_LEFT_CONTROL)) position_.y -= speed;
    }

    Camera3D Camera() const
    {
        Camera3D c = {};
        c.position = position_;
        c.target = Vector3Add(position_, Forward());
        c.up = Vector3{0.0f, 1.0f, 0.0f};
        c.fovy = 60.0f;
        c.projection = CAMERA_PERSPECTIVE;
        return c;
    }

private:
    Vector3 Forward() const
    {
        return Vector3{std::cos(pitch_) * std::sin(yaw_), std::sin(pitch_), std::cos(pitch_) * std::cos(yaw_)};
    }

    Vector3 position_{};
    float yaw_ = 0.0f;
    float pitch_ = 0.0f;
};
}

#endif
