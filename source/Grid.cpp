#include "Grid.h"
#include "defines.h"

#ifdef USE_MICROPROFILE
#include "microprofile.h"
#endif

using namespace std;
using namespace PP2;

Grid* Grid::instance = nullptr;

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

/*vector<Tank*> Grid::GetTanksAtPos(vec2<int> tankPos)
{
#ifdef USE_MICROPROFILE
    MICROPROFILE_SCOPEI("Grid", "GetTanksAtPos", MP_RED);
#endif
    vector<Tank*> ts;
    ts->clear();
    const vec2<int> checkCoords[9] = {
        {0, 0},
        {0, 1},
        {1, 1},
        {1, 0},
        {1, -1},
        {0, -1},
        {-1, -1},
        {-1, 0},
        {-1, 1}};

    for (auto c : checkCoords)
    {
        int x = tankPos.x + c.x;
        int y = tankPos.y + c.y;
        if (x >= 0 && y >= 0 && x <= GRID_SIZE_X && y <= GRID_SIZE_Y)
    }

    return ts;
}*/

void Grid::AddTankToGridCell(Tank* tank)
{
    grid[tank->gridCell.x][tank->gridCell.y].emplace_back(tank);
}

void Grid::MoveTankToGridCell(PP2::Tank* tank, vec2<int> newpos)
{
#ifdef USE_MICROPROFILE
    MICROPROFILE_SCOPEI("Grid", "MoveTankToGridCell", MP_RED);
#endif
    auto c = grid[tank->gridCell.x][tank->gridCell.y];
    /*std::vector<Tank*> k(c);
    c.clear();

    for(auto t : k){
        if(t != tank)
            c.emplace_back(t);
    }*/
    c.erase(std::remove(begin(c), end(c), tank), end(c));
    grid[newpos.x][newpos.y].emplace_back(tank);

#ifdef USE_MICROPROFILE
    MICROPROFILE_COUNTER_SET("Grid/gird/", c.size());
#endif
}
