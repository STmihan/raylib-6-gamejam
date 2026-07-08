#ifndef VIEW_RENDER_CAVITY_PARAMS_H
#define VIEW_RENDER_CAVITY_PARAMS_H

namespace view
{
struct CavityParams
{
    bool enabled = true;
    float radius = 1.0f;
    float valley = 0.6f;
    float ridge = 0.2f;
};
}

#endif
