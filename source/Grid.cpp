#include "Grid.h"
#include "defines.h"
#include <iostream>
#include <mutex>

using namespace std;
using namespace PP2;

Grid* Grid::instance = nullptr;
mutex mtx2;

Grid::Grid()
{
    for (auto& x : grid)
        for (auto& y : x) y.reserve(500);
}

Grid::~Grid() = default;

Grid* Grid::Instance()
{
    if (instance == nullptr) instance = new Grid();
    return instance;
}

vec2<int> Grid::GetGridCell(const vec2<>& position)
{
    int x = std::clamp((0.05f * position.x) * (GRID_SIZE / 100.f), -(float)GRID_OFFSET, (float)GRID_SIZE - GRID_OFFSET);
    int y = std::clamp((0.05f * position.y) * (GRID_SIZE / 100.f), -(float)GRID_OFFSET, (float)GRID_SIZE - GRID_OFFSET);

    return vec2<int>(x + GRID_OFFSET, y + GRID_OFFSET);
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
        {-1, 1}};
    return cells;
}

void Grid::AddTankToGridCell(Tank* tank) { grid[tank->gridCell.x][tank->gridCell.y].emplace_back(tank); }

void Grid::MoveTankToGridCell(PP2::Tank* tank, const vec2<int>& newPos)
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
