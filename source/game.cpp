using namespace std;

#include "Algorithms.h"
#include "template.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_FontCache.h>
#include <iostream>
#include <mutex>
#include <string>
#include <tbb/parallel_for.h>
#include <tbb/task_group.h>

using namespace PP2;

#include "Algorithms.h"
#include "Grid.h"
#include "defines.h"
#include "explosion.h"
#include "game.h"
#include "particle_beam.h"
#include "rocket.h"
#include "smoke.h"
#include "tank.h"

#ifdef USING_EASY_PROFILER

#include <easy/profiler.h>

#define PROFILE_PARALLEL 1
#endif

static timer perf_timer;
static float duration;

//Load sprite files and initialize sprites
static SDL_Surface* background_img = SDL_LoadBMP("assets/Background_Grass.bmp");
static SDL_Surface* tank_red_img = SDL_LoadBMP("assets/Tank_Proj2.bmp");
static SDL_Surface* tank_blue_img = SDL_LoadBMP("assets/Tank_Blue_Proj2.bmp");
static SDL_Surface* rocket_red_img = SDL_LoadBMP("assets/Rocket_Proj2.bmp");
static SDL_Surface* rocket_blue_img = SDL_LoadBMP("assets/Rocket_Blue_Proj2.bmp");
static SDL_Surface* particle_beam_img = SDL_LoadBMP("assets/Particle_Beam.bmp");
static SDL_Surface* smoke_img = SDL_LoadBMP("assets/Smoke.bmp");
static SDL_Surface* explosion_img = SDL_LoadBMP("assets/Explosion.bmp");

FC_Font* GameFont;

SDL_Texture* tankThreads;
SDL_Texture* tank_red;
SDL_Texture* tank_blue;
SDL_Texture* rocket_red;
SDL_Texture* rocket_blue;
SDL_Texture* smoke;
SDL_Texture* explosion;
SDL_Texture* particle_beam_sprite;

const static vec2<> tank_size(14, 18);
const static vec2<> rocket_size(25, 24);

const static float tank_radius = 12.f;
const static float rocket_radius = 10.f;

vector<int> redHealthBars = {};
vector<int> blueHealthBars = {};
vector<SDL_Point> drawPoints = {};

#define LOAD_TEX(_FIELD_) SDL_CreateTextureFromSurface(screen, _FIELD_);

mutex tankVectorMutex;
typedef unsigned int Pixel; // unsigned int is assumed to be 32-bit, which seems a safe assumption.

// subtractive blending
Pixel SubBlend(Pixel a_Color1, Pixel a_Color2)
{
    int red = (a_Color1 & REDMASK) - (a_Color2 & REDMASK);
    int green = (a_Color1 & GREENMASK) - (a_Color2 & GREENMASK);
    int blue = (a_Color1 & BLUEMASK) - (a_Color2 & BLUEMASK);
    if (red < 0) red = 0;
    if (green < 0) green = 0;
    if (blue < 0) blue = 0;
    return (Pixel)(red + green + blue);
}

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
    //initiate grid to allocate memory
    auto instance = Grid::Instance();

    tankThreads = SDL_CreateTexture(screen, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCRWIDTH, SCRHEIGHT);

    tank_red = LOAD_TEX(tank_red_img);
    tank_blue = LOAD_TEX(tank_blue_img);
    rocket_red = LOAD_TEX(rocket_red_img);
    rocket_blue = LOAD_TEX(rocket_blue_img);
    smoke = LOAD_TEX(smoke_img);
    explosion = LOAD_TEX(explosion_img);
    particle_beam_sprite = LOAD_TEX(particle_beam_img);

    GameFont = FC_CreateFont();
    FC_LoadFont(GameFont, screen, "assets/digital-7.ttf", 72, FC_MakeColor(255, 255, 255, 255), TTF_STYLE_NORMAL);

    Uint32* pixels = nullptr;
    int pitch = 0;
    // Now let's make our "pixels" pointer point to the texture data.
    SDL_LockTexture(tankThreads, nullptr, (void**)&pixels, &pitch);
    memcpy(pixels, background_img->pixels, SCRWIDTH * SCRHEIGHT * 4);
    SDL_UnlockTexture(tankThreads);

    tanks.reserve(NUM_TANKS_BLUE + NUM_TANKS_RED);
    blueTanks.reserve(NUM_TANKS_BLUE);
    redTanks.reserve(NUM_TANKS_RED);

    uint rows = (uint)sqrt(NUM_TANKS_BLUE + NUM_TANKS_RED);
    uint max_rows = 12;

    float start_blue_x = tank_size.x + 10.0f;
    float start_blue_y = tank_size.y + 80.0f;

    float start_red_x = 980.0f;
    float start_red_y = 100.0f;

    float spacing = 15.0f;

    //Spawn blue tanks
    for (int i = 0; i < NUM_TANKS_BLUE; i++)
    {
        tanks.emplace_back(start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing), BLUE,
                           tank_blue, smoke, 1200, 600, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED);
    }
    //Spawn red tanks
    for (int i = 0; i < NUM_TANKS_RED; i++)
    {
        tanks.emplace_back(start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing), RED,
                           tank_red,
                           smoke, 80, 80, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED);
    }

    particle_beams.emplace_back(vec2<>(SCRWIDTH / 2, SCRHEIGHT / 2), vec2<>(100, 50), particle_beam_sprite,
                                PARTICLE_BEAM_HIT_VALUE);
    particle_beams.emplace_back(vec2<>(80, 80), vec2<>(100, 50), particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE);
    particle_beams.emplace_back(vec2<>(1200, 600), vec2<>(100, 50), particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE);

    for (auto& tank : tanks)
    {
        instance->AddTankToGridCell(&tank);
        if (tank.alliance == RED)
            redTanks.emplace_back(&tank);
        else
            blueTanks.emplace_back(&tank);
    }

    //    blue_KD_Tree = new KD_Tree(blueTanks);
    //    blue_KD_Tree->printTree();
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown() {}

Game::~Game()
{
    //delete frame_count_font;
    FC_FreeFont(GameFont);
}

// -----------------------------------------------------------
// Update the game state:
// Move all objects
// Update sprite frames
// Collision detection
// Targeting etc..
// -----------------------------------------------------------
void Game::Update(float deltaTime)
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    if (frame_count % 200 == 0)
    {
        BuildKDTree();
    }

    //Update particle beams
    UpdateParticleBeams();

    //Update smoke plumes
    UpdateSmoke();

    //Update explosion sprites
    UpdateExplosions();

    //Remove when done with remove erase idiom
    explosions.erase(std::remove_if(explosions.begin(), explosions.end(),
                                    [](const Explosion& eExplosion) { return eExplosion.done(); }),
                     explosions.end());

    //Update rockets
    UpdateRockets();

    //Remove exploded rockets with remove erase idiom
    rockets.erase(
        std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& rocket) { return !rocket.active; }),
        rockets.end());

    tbb::task_group update_Group;
    //Update tanks
    update_Group.run([&] { UpdateTanks(); });
    update_Group.run([&] {
#ifdef USING_EASY_PROFILER
        EASY_BLOCK("UpdateRedHP", profiler::colors::Red);
#endif
        //redHealthBars = LinkedList<int>::Sort(redTanks, 100);
        redHealthBars = CountSort(redTanks);
    });
    update_Group.run([&] {
#ifdef USING_EASY_PROFILER
        EASY_BLOCK("UpdateBlueHP", profiler::colors::Blue);
#endif
        //blueHealthBars = LinkedList<int>::Sort(blueTanks, 100);
        blueHealthBars = CountSort(blueTanks);
    });
    update_Group.wait();
}

void Game::BuildKDTree()
{
#ifdef USING_EASY_PROFILER
    EASY_BLOCK("BuildKDTree", profiler::colors::Black);
#endif
    tbb::task_group KD_sort_group;
    KD_sort_group.run([&] { red_KD_Tree = new KD_Tree(redTanks); });
    KD_sort_group.run([&] { blue_KD_Tree = new KD_Tree(blueTanks); });
    KD_sort_group.wait();
}

void Game::UpdateTanks()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    tbb::parallel_for(tbb::blocked_range<int>(0, tanks.size()),
                      [&](tbb::blocked_range<int> r) {
#if PROFILE_PARALLEL == 1
                          EASY_BLOCK("Update Tank", profiler::colors::Gold);
#endif
                          for (int i = r.begin(); i < r.end(); ++i)
                          //for (int i = 0; i < tanks.size(); ++i)
                          {
                              Tank& tank = tanks[i];
                              if (!tank.active) continue;

                              //Check tank collision and nudge tanks away from each other
                              for (const auto& cell : Grid::GetNeighbouringCells())
                              {
                                  int x = tank.gridCell.x + cell.x;
                                  int y = tank.gridCell.y + cell.y;
                                  if (x < 0 || y < 0 || x > GRID_SIZE || y > GRID_SIZE) continue;

                                  for (auto& oTank : Grid::Instance()->grid[x][y])
                                  {
                                      if (&tank == oTank) continue;

                                      vec2<> dir = tank.Get_Position() - oTank->Get_Position();

                                      float colSquaredLen =
                                          (tank.Get_collision_radius() * tank.Get_collision_radius()) +
                                          (oTank->Get_collision_radius() * oTank->Get_collision_radius());

                                      if (dir.sqrLength() < colSquaredLen) tank.Push(dir.normalized(), 1.f);
                                  }
                              }

                              //Check if inside particle beam
                              for (Particle_beam& particle_beam : particle_beams)
                              {
                                  if (particle_beam.rectangle.intersectsCircle(tank.Get_Position(),
                                                                               tank.Get_collision_radius()))
                                  {
                                      if (tank.hit(particle_beam.damage))
                                      {
                                          smokes.emplace_back(smoke, tank.position - vec2<>(0, 48));
                                      }
                                  }
                              }

                              //Move tanks according to speed and nudges (see above) also reload
                              tank.Tick();

                              //Shoot at closest target if reloaded
                              if (!tank.Rocket_Reloaded()) continue;
                              Tank* target = tank.alliance == RED ? blue_KD_Tree->findClosestTank(&tank) : red_KD_Tree->findClosestTank(&tank);
                              scoped_lock lock2(tankVectorMutex);
                              rockets.emplace_back(tank.position,
                                                   (target->position - tank.position).normalized() * 3,
                                                   rocket_radius,
                                                   tank.alliance,
                                                   ((tank.alliance == RED) ? rocket_red : rocket_blue));
                              tank.Reload_Rocket();
                          }
                      });
}

void Game::UpdateSmoke()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    for (Smoke& uSmoke : smokes)
    {
        uSmoke.Tick();
    }
}

void Game::UpdateRockets()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    tbb::parallel_for(tbb::blocked_range<int>(0, rockets.size()),
                      [&](tbb::blocked_range<int> r) {
#if PROFILE_PARALLEL == 1
                          EASY_BLOCK("Update Rocket", profiler::colors::Gold);
#endif
                          for (int i = r.begin(); i < r.end(); ++i)
                          {
                              Rocket& uRocket = rockets[i];
                              uRocket.Tick();

                              if (uRocket.position.x < -250 || uRocket.position.y < -250 || uRocket.position.x > 1750 || uRocket.position.y > 1750)
                              {
                                  uRocket.active = false;
                                  continue;
                              }

                              //Check if rocket collides with enemy tank, spawn explosion and if tank is destroyed spawn a smoke plume
                              for (const auto& cell : Grid::GetNeighbouringCells())
                              {
                                  vec2<int> rocketGridCell = Grid::GetGridCell(uRocket.position);
                                  int x = rocketGridCell.x + cell.x;
                                  int y = rocketGridCell.y + cell.y;
                                  if (x < 0 || y < 0 || x > GRID_SIZE || y > GRID_SIZE) continue;

                                  for (auto& tank : Grid::Instance()->grid[x][y])
                                  {
                                      if (tank->active && (tank->alliance != uRocket.allignment) &&
                                          uRocket.Intersects(tank->position, tank->collision_radius))
                                      {
                                          scoped_lock lock(tankVectorMutex);
                                          explosions.emplace_back(explosion, tank->position);

                                          if (tank->hit(ROCKET_HIT_VALUE))
                                          {
                                              smokes.emplace_back(smoke, tank->position - vec2<>(0, 48));
                                          }

                                          uRocket.active = false;
                                          break;
                                      }
                                  }
                              }
                          }
                      });
#ifdef USING_EASY_PROFILER
    //MICROPROFILE_COUNTER_SET("Game/rockets/", rockets.size());
#endif
}

void Game::UpdateParticleBeams()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    for (Particle_beam& particle_beam : particle_beams)
    {
        particle_beam.tick();
    }
}

void Game::UpdateExplosions()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    for (Explosion& uExplosion : explosions)
    {
        uExplosion.Tick();
    }
}

void Game::Draw()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif

    //Draw background
    //SDL_RenderCopy(screen, background, NULL, NULL);
    SDL_RenderCopy(screen, tankThreads, NULL, NULL);

#ifdef USING_EASY_PROFILER
    EASY_BLOCK("Draw tanks", profiler::colors::Red);
#endif
    Pixel* pixels = nullptr;
    int pitch = 0;
    // Now let's make our "pixels" pointer point to the texture data.
    SDL_LockTexture(tankThreads, nullptr, (void**)&pixels, &pitch);

    try
    {
        //Draw sprites
        for (int i = 0; i < MAX_TANKS; i++)
        {
            tanks.at(i).Draw(screen);

            vec2 tPos = tanks.at(i).Get_Position();
            // tread marks
            if ((tPos.x >= 0) && (tPos.x < SCRWIDTH) && (tPos.y >= 0) && (tPos.y < SCRHEIGHT))
            {
                // Before setting the color, we need to know where we have to place it.
                Pixel pixelPosition = (int)tPos.y * (pitch / sizeof(Pixel)) + (int)tPos.x;
                // Now we can set the pixel(s) we want.
                //pixels[pixelPosition] *= 0x808080; //Black;
                pixels[pixelPosition] = SubBlend(pixels[pixelPosition], 0x80808080);
            }
        }
    }
    catch (...)
    {
    }

    // Also don't forget to unlock your texture once you're done.
    SDL_UnlockTexture(tankThreads);

#ifdef USING_EASY_PROFILER
    EASY_END_BLOCK
#endif

    for (Rocket& r : rockets)
    {
        r.Draw(screen);
    }

    for (Smoke& s : smokes) { s.Draw(screen); }

    for (Particle_beam& b : particle_beams) { b.Draw(screen); }

    for (Explosion& e : explosions) { e.Draw(screen); }

    SDL_Rect red = {0, 0, SCRWIDTH, HEALTH_BAR_HEIGHT};
    SDL_Rect blue = {0, (SCRHEIGHT - HEALTH_BAR_HEIGHT) - 1, SCRWIDTH, SCRWIDTH - 1};
    SDL_SetRenderDrawColor(screen, 0, 255, 0, 255);
    SDL_RenderFillRect(screen, &red);
    SDL_RenderFillRect(screen, &blue);

    SDL_SetRenderDrawColor(screen, 255, 0, 0, 255);

#ifdef USING_EASY_PROFILER
    EASY_BLOCK("Draw Health_Bar_Red", profiler::colors::Red);
#endif
    //Draw sorted health bars red tanks

    int countRed = 0;
    for (int currentRedTank : redHealthBars)
    {
        DrawTankHP(countRed, RED, currentRedTank);
        countRed++;
    }

    if (!drawPoints.empty()) SDL_RenderDrawLines(screen, &drawPoints[0], drawPoints.size());
    drawPoints.clear();

#ifdef USING_EASY_PROFILER
    EASY_END_BLOCK
    EASY_BLOCK("Draw Health_Bar_Blue", profiler::colors::Blue);
#endif
    //Draw sorted health bars blue tanks
    int countBlue = 0;
    for (int currentBlueTank : blueHealthBars)
    {
        DrawTankHP(countBlue, BLUE, currentBlueTank);
        countBlue++;
    }
    if (!drawPoints.empty()) SDL_RenderDrawLines(screen, &drawPoints[0], drawPoints.size());
    drawPoints.clear();
#ifdef USING_EASY_PROFILER
    EASY_END_BLOCK
#endif
}

void Game::DrawTankHP(int i, alliances al, int health)
{
    if (health == TANK_MAX_HEALTH) return;

    int health_bar_start_x = i * (HEALTH_BAR_WIDTH + HEALTH_BAR_SPACING) + HEALTH_BARS_OFFSET_X;
    int health_bar_start_y = (al == BLUE) ? 0 : (SCRHEIGHT - HEALTH_BAR_HEIGHT) - 1;
    int health_bar_end_x = health_bar_start_x + HEALTH_BAR_WIDTH;
    int health_bar_end_y = (al == BLUE) ? 0 : (SCRHEIGHT - HEALTH_BAR_HEIGHT) - 1;

    drawPoints.emplace_back(SDL_Point{health_bar_start_x, health_bar_start_y});
    drawPoints.emplace_back(
        SDL_Point{health_bar_end_x, health_bar_end_y + (int)((double)HEALTH_BAR_HEIGHT * (1 - ((double)health /
                                                                                               (double)TANK_MAX_HEALTH)))});
}

// -----------------------------------------------------------
// When we reach MAX_FRAMES print the duration and speedup multiplier
// Updating REF_PERFORMANCE at the top of this file with the value
// on your machine gives you an idea of the speedup your optimizations give
// -----------------------------------------------------------
void PP2::Game::MeasurePerformance()
{
    char buffer[128];
    if (frame_count >= MAX_FRAMES)
    {
        if (!lock_update)
        {
            lock_update = true;
            duration = perf_timer.elapsed();
            cout << "Duration was: " << duration << " (Replace REF_PERFORMANCE with this value)" << endl;
        }

        frame_count--;
    }

    if (lock_update)
    {
        SDL_Rect r = {420, 170, 450, 260};
        SDL_SetRenderDrawColor(screen, 0, 0, 0, 255);
        SDL_RenderFillRect(screen, &r);

        int ms = (int)duration % 1000, sec = ((int)duration / 1000) % 60, min = ((int)duration / 60000);
        FC_Draw(GameFont, screen, 470, 220, " %02i:%02i:%03i \n SPEEDUP: %4.1f", min, sec, ms, REF_PERFORMANCE / duration);
    }
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick(float deltaTime)
{
    if (!lock_update)
        Update(deltaTime);

    Draw();

    MeasurePerformance();

#if PROFILE_PARALLEL == 1
    EASY_BLOCK("Print frame count", profiler::colors::Gold);
#endif
    if (!lock_update)
    {
        //Print frame count
        frame_count++;
        FC_Draw(GameFont, screen, 5, 5, "%lld", frame_count);
    }
}
