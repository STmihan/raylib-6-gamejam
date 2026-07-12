#include "app/core/audio_events.h"

#include "audio/sound.h"
#include "data/unit/unit_types.h"
#include "logic/state/game_state.h"

namespace app
{
void AudioEvents::Reset()
{
    beamSeen_.fill(0);
    healSeen_.fill(0);
    projActive_.fill(false);
    projType_.fill(-1);
}

void AudioEvents::Observe(const logic::GameState& state)
{
    if (!init_ || state.tick < lastTick_)
    {
        Reset();
        init_ = true;
    }
    lastTick_ = state.tick;

    for (int i = 0; i < data::MaxBeams; i++)
    {
        const logic::Beam& beam = state.beams[i];
        if (!beam.active || beamSeen_[i] == beam.startTick) continue;
        beamSeen_[i] = beam.startTick;
        if (beam.attackerSlot < 0 || beam.attackerSlot >= data::MaxEntities) continue;
        const logic::Entity& attacker = state.entities[beam.attackerSlot];
        if (attacker.kind == logic::EntityKind::Unit &&
            (attacker.type == data::UnitType::Infantry || attacker.type == data::UnitType::Plane))
            audio::Play("gun-hit");
    }

    for (int i = 0; i < data::MaxProjectiles; i++)
    {
        const logic::Projectile& proj = state.projectiles[i];
        if (proj.active)
        {
            if (!projActive_[i])
            {
                projActive_[i] = true;
                projType_[i] = (proj.attackerSlot >= 0 && proj.attackerSlot < data::MaxEntities)
                    ? static_cast<int>(state.entities[proj.attackerSlot].type)
                    : -1;
            }
        }
        else if (projActive_[i])
        {
            projActive_[i] = false;
            if (projType_[i] == static_cast<int>(data::UnitType::Rocketeer)) audio::Play("rocket-hit");
            projType_[i] = -1;
        }
    }

    for (int i = 0; i < data::MaxHealPulses; i++)
    {
        const logic::HealPulse& pulse = state.healPulses[i];
        if (!pulse.active || healSeen_[i] == pulse.startTick) continue;
        healSeen_[i] = pulse.startTick;
        audio::Play("heal");
    }
}
}
