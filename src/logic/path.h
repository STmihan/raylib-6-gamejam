#ifndef LOGIC_PATH_H
#define LOGIC_PATH_H

#include <cstdint>

#include "logic/world/map.h"

namespace logic
{
bool Passable(const Map& map, const int* occupant, int selfIndex, int col, int row);

bool FindStep(const Map& map, const int* occupant, int selfIndex, int fromCol, int fromRow, int goalCol,
              int goalRow, int stopRange, int& outCol, int& outRow);

std::uint64_t FindStepCalls();
void ResetFindStepCalls();
}

#endif
