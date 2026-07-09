#ifndef VIEW_ANIM_HIT_FLASH_H
#define VIEW_ANIM_HIT_FLASH_H

#include "raylib.h"

namespace view
{
inline constexpr float HitFlashDuration = 0.18f;

inline Color HitFlashTint(float intensity)
{
    intensity = intensity < 0.0f ? 0.0f : (intensity > 1.0f ? 1.0f : intensity);
    unsigned char c = static_cast<unsigned char>(255.0f - 195.0f * intensity);
    return Color{255, c, c, 255};
}
}

#endif
