#ifndef DATA_ECONOMY_ECONOMY_H
#define DATA_ECONOMY_ECONOMY_H

namespace data
{
inline constexpr float ResourceCap = 6.0f;
inline constexpr float BaseRegenPerSec = 0.5f;
inline constexpr float DeployFreezeSeconds = 1.0f;

inline constexpr int EngineerHealDeployRadius = 2;
inline constexpr float EngineerHealDeployFraction = 0.4f;
inline constexpr int EngineerHealPulseRadius = 1;
inline constexpr int EngineerHealPulseAmount = 10;
inline constexpr float EngineerHealPulseInterval = 2.0f;
inline constexpr float HealWaveSeconds = 0.55f;

inline constexpr int TankArmorHits = 3;

inline constexpr int ForestMissPercent = 30;

inline constexpr int BaseTurretRange = 4;
inline constexpr int BaseTurretDamage = 25;
inline constexpr float BaseTurretInterval = 0.8f;

inline constexpr int MatchDurationSeconds = 180;
inline constexpr int RegenStepSeconds = 60;
inline constexpr int MaxRegenDoublings = 2;

inline constexpr int OvertimeBaseDamage = 10;
inline constexpr int OvertimeDoubleEverySeconds = 30;

inline float RegenPerSec(float elapsedSeconds)
{
    int doublings = static_cast<int>(elapsedSeconds / static_cast<float>(RegenStepSeconds));
    if (doublings > MaxRegenDoublings) doublings = MaxRegenDoublings;
    return BaseRegenPerSec * static_cast<float>(1 << doublings);
}

inline int OvertimeDamageAt(int overtimeSeconds)
{
    int doublings = overtimeSeconds / OvertimeDoubleEverySeconds;
    return OvertimeBaseDamage << doublings;
}
}

#endif
