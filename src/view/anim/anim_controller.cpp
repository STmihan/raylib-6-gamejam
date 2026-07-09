#include "view/anim/anim_controller.h"

#include "raylib.h"

#include "data/unit/unit.h"

namespace view
{
namespace
{
    constexpr float MoveEpsilonSq = 0.25f;

    AnimState DeriveState(const logic::Entity& previous, const logic::Entity& current)
    {
        if (current.targetSlot >= 0) return AnimState::Attack;
        float dx = current.position.x - previous.position.x;
        float dy = current.position.y - previous.position.y;
        if (previous.active && dx * dx + dy * dy > MoveEpsilonSq) return AnimState::Move;
        return AnimState::Idle;
    }

    const char* StructureName(logic::EntityKind kind)
    {
        return kind == logic::EntityKind::Base ? "Base" : "Wall";
    }
}

void AnimController::Update(const logic::GameState& previous, const logic::GameState& current)
{
    for (int i = 0; i < data::MaxEntities; i++)
    {
        const logic::Entity& cur = current.entities[i];
        const logic::Entity& prev = previous.entities[i];

        if (cur.active)
        {
            if (cur.kind == logic::EntityKind::Unit)
            {
                AnimState state = DeriveState(prev, cur);
                if (!tracked_[i] || state_[i] != state)
                {
                    TraceLog(LOG_INFO, "[anim] %s#%u: %s", data::UnitTypeName(cur.type), cur.id,
                             AnimStateName(state));
                    state_[i] = state;
                }
            }
            tracked_[i] = true;
            active_[i] = true;
            continue;
        }

        if (active_[i])
        {
            if (prev.kind == logic::EntityKind::Unit)
            {
                TraceLog(LOG_INFO, "[anim] %s#%u: %s", data::UnitTypeName(prev.type), prev.id,
                         AnimStateName(AnimState::Death));
            }
            else
            {
                TraceLog(LOG_INFO, "[anim] %s#%u: destroy", StructureName(prev.kind), prev.id);
            }
        }
        tracked_[i] = false;
        active_[i] = false;
    }
}
}
