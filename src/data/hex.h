#ifndef DATA_HEX_H
#define DATA_HEX_H

#include "data/vec.h"
#include "data/world_config.h"

namespace data {

inline Vec2 CellToLogic(int q, int r) {
    float x = ColSpacingLogic * static_cast<float>(q);
    float y = RowSpacingLogic * static_cast<float>(r) + OddColumnOffsetLogic * static_cast<float>(q & 1);
    return { x, y };
}

inline Vec2 FieldCenterLogic() {
    return {
        ColSpacingLogic * static_cast<float>(FieldCols - 1) * 0.5f,
        RowSpacingLogic * static_cast<float>(FieldRows - 1) * 0.5f,
    };
}

inline Vec2 FieldExtentLogic() {
    return {
        ColSpacingLogic * static_cast<float>(FieldCols - 1),
        RowSpacingLogic * static_cast<float>(FieldRows - 1) + OddColumnOffsetLogic,
    };
}

}

#endif
