#ifndef DATA_TIME_CONFIG_H
#define DATA_TIME_CONFIG_H

namespace data {

inline constexpr int TickRate = 20;
inline constexpr double TickDelta = 1.0 / TickRate;
inline constexpr int MaxTicksPerFrame = 5;

}

#endif
