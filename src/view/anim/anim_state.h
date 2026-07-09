#ifndef VIEW_ANIM_ANIM_STATE_H
#define VIEW_ANIM_ANIM_STATE_H

namespace view
{
enum class AnimState
{
    Idle,
    Move,
    Attack,
    Death,
};

inline const char* AnimStateName(AnimState state)
{
    switch (state)
    {
    case AnimState::Idle: return "idle";
    case AnimState::Move: return "move";
    case AnimState::Attack: return "attack";
    case AnimState::Death: return "death";
    }
    return "idle";
}
}

#endif
