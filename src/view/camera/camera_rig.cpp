#include "view/camera/camera_rig.h"

#include <cmath>

#include "data/space/hex.h"
#include "data/space/world_config.h"

namespace view
{
namespace
{
    float Clamp(float value, float low, float high)
    {
        if (value < low) return low;
        if (value > high) return high;
        return value;
    }
}

void CameraRig::Init()
{
    data::Vec2 center = data::FieldCenterLogic();
    data::Vec2 extent = data::FieldExtentLogic();

    focus_ = {center.x * data::RenderScale, 0.0f, center.y * data::RenderScale};
    offset_ = {-8.0f, 16.0f, 13.0f};
    panSpeed_ = 0.03f;

    float margin = 1.0f;
    boundsMin_ = {-margin, -margin};
    boundsMax_ = {extent.x * data::RenderScale + margin, extent.y * data::RenderScale + margin};
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
    focus_.x = Clamp(focus_.x, boundsMin_.x, boundsMax_.x);
    focus_.z = Clamp(focus_.z, boundsMin_.y, boundsMax_.y);
}

Camera3D CameraRig::Camera() const
{
    Camera3D camera = {};
    camera.position = {focus_.x + offset_.x, focus_.y + offset_.y, focus_.z + offset_.z};
    camera.target = focus_;
    camera.up = {0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
}
}
