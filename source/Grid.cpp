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

// 0.05f => 100 / 2000 (assuming tanks wont go outside [x](-200, 1800), [y](-200, 1800))
// clamp makes sure tanks are always inside the grid, if the go outside it the will be set back to the min/max grid cell values
#define CLAP_POS(_IN_) std::clamp((0.05f * _IN_) * (GRID_SIZE / 100.f), -(float)GRID_OFFSET, (float)GRID_SIZE - GRID_OFFSET) + GRID_OFFSET
vec2<int> Grid::GetGridCell(const vec2<>& position)
{
    return vec2<int>(CLAP_POS(position.x), CLAP_POS(position.y));
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
