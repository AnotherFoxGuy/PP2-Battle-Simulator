#include <iostream>
#include <mutex>
#include "Grid.h"
#include "defines.h"

using namespace std;
using namespace PP2;

Grid* Grid::instance = nullptr;
mutex mtx2;

Grid::Grid() { for (auto& x : grid) for (auto& y : x) y.reserve(500); }

Grid::~Grid() = default;

Grid* Grid::Instance()
{
    if (instance == nullptr) instance = new Grid();
    return instance;
}

vec2<int> Grid::GetGridCell(vec2<> position)
{
    int x = std::clamp(position.x / GRID_SIZE_X, 0.0f, (float)GRID_SIZE_X);
    int y = std::clamp(position.y / GRID_SIZE_Y, 0.0f, (float)GRID_SIZE_Y);

    return vec2<int>(x, y);
}


vector<vec2<int>> Grid::GetNeighbouringCells()
{
    vector<vec2<int>> cells = {
        {0, 0},
        {0, 1},
        {1, 1},
        {1, 0},
        {1, -1},
        {0, -1},
        {-1, -1},
        {-1, 0},
        {-1, 1}
    };
    return cells;
}

void Grid::AddTankToGridCell(Tank* tank) { grid[tank->gridCell.x][tank->gridCell.y].emplace_back(tank); }

void Grid::MoveTankToGridCell(PP2::Tank* tank, vec2<int> newPos)
{
    scoped_lock lock(mtx2);
    auto& gridCell = grid[tank->gridCell.x][tank->gridCell.y];
    grid[newPos.x][newPos.y].emplace_back(tank);
    for (int i = 0; i < gridCell.size(); ++i)
    {
        if (gridCell[i] == tank)
        {
            gridCell.erase(gridCell.begin() + i);
            break;
        }
    }
}
