#include "logic/world/map.h"

namespace logic {

Map BuildMap() {
    Map map{};

    int row = 0;
    int col = 0;
    for (const char *symbol = data::FieldLayout; *symbol != '\0'; ++symbol) {
        if (*symbol == '\n') {
            if (col > 0) row++;
            col = 0;
            continue;
        }
        if (map.InBounds(col, row)) {
            map.Set(col, row, data::TileFromChar(*symbol));
        }
        col++;
    }

    return map;
}

}
