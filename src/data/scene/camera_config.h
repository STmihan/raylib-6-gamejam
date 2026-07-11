#ifndef DATA_SCENE_CAMERA_CONFIG_H
#define DATA_SCENE_CAMERA_CONFIG_H

#include "raylib.h"

namespace data
{
inline constexpr Vector3 CameraOffset = {-16.0f, 32.0f, 26.0f};
inline constexpr float CameraPanSpeed = 0.05f;
inline constexpr float CameraBoundsRadius = 18.0f;
inline constexpr float CameraFovy = 45.0f;
}

#endif
