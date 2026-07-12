#ifndef VIEW_PREFAB_DAMAGE_NUMBERS_H
#define VIEW_PREFAB_DAMAGE_NUMBERS_H

#include <array>
#include <cstdint>

#include "raylib.h"

#include "data/sim/sim_config.h"
#include "data/space/vec.h"

namespace logic { struct GameState; }
namespace view::ui { class UiContext; }

namespace view
{
class DamageNumbers
{
public:
    void Update(const logic::GameState& current, float dt);
    void Draw(ui::UiContext& ui, Camera3D camera) const;

private:
    static constexpr int Max = 48;
    struct Popup
    {
        bool active = false;
        int owner = -1;
        data::Vec2 origin{};
        int amount = 0;
        bool heal = false;
        bool miss = false;
        float mult = 1.0f;
        float age = 0.0f;
        char text[12] = {0};
    };

    int FindSlot();
    void Reset();

    std::array<Popup, Max> pool_{};
    std::array<int, data::MaxEntities> lastHp_{};
    std::array<std::uint32_t, data::MaxEntities> lastId_{};
    std::array<int, data::MaxEntities> recent_{};
    std::array<std::uint64_t, data::MaxMisses> missSeen_{};
    std::uint64_t lastTick_ = 0;
    bool init_ = false;
};
}

#endif
