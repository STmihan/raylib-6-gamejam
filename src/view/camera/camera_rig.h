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

    float& BoundsRadiusRef() { return boundsRadius_; }

private:
    Vector3 focus_{};
    Vector3 offset_{};
    Vector2 center_{};
    float boundsRadius_{};
    float panSpeed_{};
};
}

#endif
