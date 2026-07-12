#ifndef APP_AUDIO_EVENTS_H
#define APP_AUDIO_EVENTS_H

#include <array>
#include <cstdint>

#include "data/sim/sim_config.h"

namespace logic { struct GameState; }

namespace app
{
class AudioEvents
{
public:
    void Observe(const logic::GameState& state);

private:
    void Reset();

    bool init_ = false;
    std::uint64_t lastTick_ = 0;
    std::array<std::uint64_t, data::MaxBeams> beamSeen_{};
    std::array<std::uint64_t, data::MaxHealPulses> healSeen_{};
    std::array<bool, data::MaxProjectiles> projActive_{};
    std::array<int, data::MaxProjectiles> projType_{};
};
}

#endif
