#include "view/camera/camera_rig.h"

#include <cmath>

#include "data/scene/camera_config.h"
#include "data/space/hex.h"
#include "data/space/world_config.h"

namespace view
{
void CameraRig::Init()
{
    data::Vec2 center = data::FieldCenterLogic();

    focus_ = {center.x * data::RenderScale, 0.0f, center.y * data::RenderScale};
    offset_ = data::CameraOffset;
    panSpeed_ = data::CameraPanSpeed;
    center_ = {focus_.x, focus_.z};
    boundsRadius_ = data::CameraBoundsRadius;
}

void CameraRig::Update()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        Vector2 delta = GetMouseDelta();

        float forwardX = -offset_.x;
        float forwardZ = -offset_.z;
        float length = std::sqrt(forwardX * forwardX + forwardZ * forwardZ);
        forwardX /= length;
        forwardZ /= length;

        float rightX = -forwardZ;
        float rightZ = forwardX;

        float moveX = rightX * delta.x - forwardX * delta.y;
        float moveZ = rightZ * delta.x - forwardZ * delta.y;

        focus_.x -= moveX * panSpeed_;
        focus_.z -= moveZ * panSpeed_;
    }
    float dx = focus_.x - center_.x;
    float dz = focus_.z - center_.y;
    float dist = std::sqrt(dx * dx + dz * dz);
    if (dist > boundsRadius_)
    {
        float scale = boundsRadius_ / dist;
        focus_.x = center_.x + dx * scale;
        focus_.z = center_.y + dz * scale;
    }
}

Camera3D CameraRig::Camera() const
{
    Camera3D camera = {};
    camera.position = {focus_.x + offset_.x, focus_.y + offset_.y, focus_.z + offset_.z};
    camera.target = focus_;
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = data::CameraFovy;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}
}
