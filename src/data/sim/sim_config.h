#ifndef DATA_SIM_SIM_CONFIG_H
#define DATA_SIM_SIM_CONFIG_H

namespace data
{
inline constexpr int MaxEntities = 256;
inline constexpr int DemoEntityCount = 8;

inline constexpr float SpawnSpeedBase = 90.0f;
inline constexpr float SpawnSpeedStep = 20.0f;
inline constexpr int SpawnSpeedTiers = 4;
inline constexpr float SpawnYOffsetStep = 40.0f;
inline constexpr int SpawnYSpread = 3;
}

#endif
