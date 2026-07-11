#ifndef DATA_SIM_SIM_CONFIG_H
#define DATA_SIM_SIM_CONFIG_H

namespace data
{
inline constexpr int MaxEntities = 256;

inline constexpr float CellReachedEpsilon = 1.0f;

inline constexpr int BaseHp = 2000;
inline constexpr int WallHp = 300;

inline constexpr int MaxProjectiles = 128;
inline constexpr int MaxHealPulses = 32;
inline constexpr int MaxBeams = 64;
inline constexpr int MaxMisses = 32;
inline constexpr float BeamSeconds = 0.1f;
inline constexpr float MissSeconds = 0.35f;
inline constexpr float PathRepathSeconds = 0.35f;
inline constexpr float ShellSpeedRocketeer = 700.0f;
inline constexpr float ShellSpeedAA = 520.0f;
inline constexpr float BurstDelay = 0.08f;
}

#endif
