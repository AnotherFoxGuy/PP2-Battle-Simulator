#pragma once

#include "defines.h"
#include "tank.h"
#include <vector>

namespace PP2
{
class Grid
{
public:
    static Grid* Instance();
    ~Grid();
    void AddTankToGridCell(Tank* tank);
    static vec2<int> GetGridCell(vec2<> position);
    void MoveTankToGridCell(Tank* tank, vec2<int> newPos);
    std::vector<vec2<int>> GetNeighbouringCells();

    std::vector<Tank*> grid[GRID_SIZE_X + 1][GRID_SIZE_Y + 1];

private:
    /* Here will be the instance stored. */
    static Grid* instance;

    /* Private constructor to prevent instancing. */
    Grid();
};
} // namespace PP2
