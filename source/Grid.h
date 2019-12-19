#pragma once

#include "tank.h"
#include <vector>
#define GRID_SIZE_X 24
#define GRID_SIZE_Y 32

namespace PP2
{
class Grid
{
  public:
    Grid();
    ~Grid();
    void AddTankToGridCell(Tank* tank);
    static vec2<int> GetGridCell(vec2<> tankPos);
    void MoveTankToGridCell(Tank* tank);

  private:
    std::vector<Tank*> grid[GRID_SIZE_X][GRID_SIZE_Y];
};
} // namespace PP2
