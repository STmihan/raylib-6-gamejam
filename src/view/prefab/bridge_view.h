#ifndef VIEW_PREFAB_BRIDGE_VIEW_H
#define VIEW_PREFAB_BRIDGE_VIEW_H

#include "raylib.h"

namespace view
{
class BridgeView
{
public:
    void Appear(Vector3 position)
    {
        // TraceLog(LOG_INFO, "[anim] Bridge (%.1f, %.1f): appear", position.x, position.z);
    }

    void Destroy(Vector3 position)
    {
        // TraceLog(LOG_INFO, "[anim] Bridge (%.1f, %.1f): destroy", position.x, position.z);
    }
};
}

#endif
