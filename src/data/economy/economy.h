#ifndef DATA_ECONOMY_ECONOMY_H
#define DATA_ECONOMY_ECONOMY_H

#include "data/balance/balance.h"

namespace data
{
inline constexpr float HealWaveSeconds = 0.55f;

inline float ResourceCap() { return Rules().resourceCap; }
inline float BaseRegenPerSec() { return Rules().baseRegenPerSec; }
inline float DeployFreezeSeconds() { return Rules().deployFreezeSeconds; }

inline int EngineerHealDeployRadius() { return Rules().engineerHealDeployRadius; }
inline float EngineerHealDeployFraction() { return Rules().engineerHealDeployFraction; }
inline int EngineerHealPulseRadius() { return Rules().engineerHealPulseRadius; }
inline int EngineerHealPulseAmount() { return Rules().engineerHealPulseAmount; }
inline float EngineerHealPulseInterval() { return Rules().engineerHealPulseInterval; }

inline int BaseTurretRange() { return Rules().baseTurretRange; }
inline int BaseTurretDamage() { return Rules().baseTurretDamage; }
inline float BaseTurretInterval() { return Rules().baseTurretInterval; }

inline int MatchDurationSeconds() { return Rules().matchDurationSeconds; }
inline int ForestMissPercent() { return Rules().forestMissPercent; }
inline float AiDeployCooldownSeconds() { return Rules().aiDeployCooldownSeconds; }

inline float RegenPerSec(float elapsedSeconds)
{
    int doublings = static_cast<int>(elapsedSeconds / static_cast<float>(Rules().regenStepSeconds));
    if (doublings > Rules().maxRegenDoublings) doublings = Rules().maxRegenDoublings;
    return Rules().baseRegenPerSec * static_cast<float>(1 << doublings);
}

inline int OvertimeDamageAt(int overtimeSeconds)
{
    int doublings = overtimeSeconds / Rules().overtimeDoubleEverySeconds;
    return Rules().overtimeBaseDamage << doublings;
}
}

#endif
