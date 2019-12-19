#include "Grid.h"

using namespace std;

PP2::Grid::Grid()
{
    for (auto& x : grid)
        for (auto& y : x)
            y.reserve(500);
}
PP2::Grid::~Grid()
{
}

PP2::vec2<int> PP2::Grid::GetGridCell(vec2<> tankPos)
{
    int x = std::clamp(tankPos.x / GRID_SIZE_X, 0.0f, (float)GRID_SIZE_X);
    int y = std::clamp(tankPos.y / GRID_SIZE_Y, 0.0f, (float)GRID_SIZE_Y);

    return vec2<int>(x, y);
}

void PP2::Grid::AddTankToGridCell(Tank* tank)
{
    grid[tank->gridCell.x][tank->gridCell.y].push_back(tank);
    /*auto g = grid[tank->gridCell.x][tank->gridCell.y];
    g.emplace_back(tank);*/
}

void PP2::Grid::MoveTankToGridCell(PP2::Tank* tank)
{
}
