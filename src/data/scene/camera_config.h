#ifndef DATA_SCENE_CAMERA_CONFIG_H
#define DATA_SCENE_CAMERA_CONFIG_H

#include "raylib.h"

namespace data
{
inline constexpr Vector3 CameraOffset = {-8.0f, 16.0f, 13.0f};
inline constexpr float CameraPanSpeed = 0.03f;
inline constexpr float CameraBoundsRadius = 9.0f;
inline constexpr float CameraFovy = 45.0f;
}

#endif
