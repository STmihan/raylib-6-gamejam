#ifndef VIEW_CAMERA_RIG_H
#define VIEW_CAMERA_RIG_H

#include "raylib.h"

namespace view
{
class CameraRig
{
public:
    void Init();
    void Update();
    Camera3D Camera() const;

private:
    Vector3 focus_{};
    Vector3 offset_{};
    Vector2 boundsMin_{};
    Vector2 boundsMax_{};
    float panSpeed_ = 0.03f;
};
}

#endif
