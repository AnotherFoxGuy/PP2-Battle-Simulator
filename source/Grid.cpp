#include "Grid.h"
#include "defines.h"
#include <mutex>

#include <easy/profiler.h>

using namespace std;
using namespace PP2;

Grid* Grid::instance = nullptr;
mutex gmtx;

Grid::Grid()
{
    for (auto& x : grid)
        for (auto& y : x)
            y.reserve(500);
}
Grid::~Grid() = default;

Grid* Grid::Instance()
{
    if (instance == nullptr)
        instance = new Grid();
    return instance;
}

vec2<int> Grid::GetGridCell(vec2<> tankPos)
{
    int x = std::clamp(tankPos.x / GRID_SIZE_X, 0.0f, (float)GRID_SIZE_X);
    int y = std::clamp(tankPos.y / GRID_SIZE_Y, 0.0f, (float)GRID_SIZE_Y);

    return vec2<int>(x, y);
}

vector<Tank*> Grid::GetTanksAtPos(vec2<int> tankPos)
{
    EASY_FUNCTION(profiler::colors::Magenta);
    std::vector<Tank*> ts;
    ts.clear();
    const vec2<int> checkCoords[9] = {
        {-1, -1},
        {0, 1},
        {1, 1},
        {-1, 0},
        {0, 0},
        {1, 0},
        {-1, -1},
        {0, -1},
        {1, -1},
    };
    scoped_lock lock(gmtx);
    for (auto c : checkCoords)
    {

        int x = tankPos.x + c.x;
        int y = tankPos.y + c.y;
        if (x >= 0 && y >= 0 && x <= GRID_SIZE_X && y <= GRID_SIZE_Y)
            ts.insert(end(ts), begin(grid[x][y]), end(grid[x][y]));
    }

    return ts;
}

void Grid::AddTankToGridCell(Tank* tank)
{
    grid[tank->gridCell.x][tank->gridCell.y].emplace_back(tank);
}

void Grid::MoveTankToGridCell(PP2::Tank* tank, vec2<int> newpos)
{
    scoped_lock lock(gmtx);
    EASY_FUNCTION(profiler::colors::Magenta);
    auto& c = grid[tank->gridCell.x][tank->gridCell.y];
    c.erase(std::remove(begin(c), end(c), tank), end(c));
    grid[newpos.x][newpos.y].push_back(tank);
    //MICROPROFILE_COUNTER_SET("Grid/gird/", c.size());
}
