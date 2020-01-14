#pragma once

#include "Grid.h"
#include "Algorithms.h"
#include "defines.h"
#include "explosion.h"
#include "particle_beam.h"
#include "rocket.h"
#include "smoke.h"
#include "tank.h"
#include <cstdint>
#include <iostream>

namespace PP2
{

class Game
{
public:
    void SetTarget(SDL_Renderer* surface) { screen = surface; }

    void Init();

    void Shutdown();

    void Update(float deltaTime);

    void Draw();

    void Tick(float deltaTime);

    void MeasurePerformance();

    Tank& FindClosestEnemy(Tank& current_tank);

    void DrawTankHP(int i, char color, int health);

    void MouseUp(int button)
    {
        /* implement if you want to detect mouse button presses */
    }

    void MouseDown(int button)
    {
        /* implement if you want to detect mouse button presses */
    }

    void MouseMove(int x, int y)
    {
        /* implement if you want to detect mouse movement */
    }

    void KeyUp(int key)
    {
        /* implement if you want to handle keys */
    }

    void KeyDown(int key)
    {
        /* implement if you want to handle keys */
    }

private:
    SDL_Renderer* screen;
    std::vector<Tank> tanks;
    std::vector<Tank*> blueTanks;
    std::vector<Tank*> redTanks;
    std::vector<Rocket> rockets;
    std::vector<Smoke> smokes;
    std::vector<Explosion> explosions;
    std::vector<Particle_beam> particle_beams;

    //Font *frame_count_font;
    long long frame_count = 0;

    bool lock_update = false;

    void UpdateTanks();

    void UpdateSmoke();

    void UpdateRockets();

    void UpdateParticleBeams();

    void UpdateExplosions();

    void SortHealthBars();

    std::vector<LinkedList> Sort(std::vector<Tank*>& input, int n_buckets);

    ~Game();
};
}; // namespace PP2
