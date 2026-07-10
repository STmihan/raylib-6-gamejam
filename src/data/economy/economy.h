#ifndef DATA_ECONOMY_ECONOMY_H
#define DATA_ECONOMY_ECONOMY_H

namespace data
{
inline constexpr float ResourceCap = 6.0f;
inline constexpr float BaseRegenPerSec = 0.5f;
inline constexpr float RegenDoubleEverySeconds = 120.0f;
inline constexpr int MaxRegenDoublings = 6;

inline float RegenPerSec(float elapsedSeconds)
{
    int doublings = static_cast<int>(elapsedSeconds / RegenDoubleEverySeconds);
    if (doublings > MaxRegenDoublings) doublings = MaxRegenDoublings;
    return BaseRegenPerSec * static_cast<float>(1 << doublings);
}
}

#endif
