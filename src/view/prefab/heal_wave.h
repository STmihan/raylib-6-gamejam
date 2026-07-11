#ifndef VIEW_PREFAB_HEAL_WAVE_H
#define VIEW_PREFAB_HEAL_WAVE_H

namespace logic { struct GameState; }

namespace view
{
class HealWaveView
{
public:
    void Draw(const logic::GameState& state, float alpha) const;
};
}

#endif
