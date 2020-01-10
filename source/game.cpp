using namespace std;

#include "surface.h"
#include "template.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <tbb/parallel_for.h>
#include <tbb/task_group.h>
#include <SDL2/SDL_ttf.h>

using namespace PP2;

#include "ThreadPool.h"
#include "Grid.h"
#include "explosion.h"
#include "particle_beam.h"
#include "rocket.h"
#include "smoke.h"
#include "tank.h"

#include "game.h"

#ifdef USING_EASY_PROFILER

#include <easy/profiler.h>

#define PROFILE_PARALLEL 0
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

TTF_Font* FPS;
TTF_Font* End;
static SDL_Color White = {255, 255, 255};
static SDL_Rect framecounter_message_rect = {50, 50, 500, 100}; //create a rect
SDL_Surface* text_surface;
SDL_Texture* text_texture;

SDL_Texture* background;
SDL_Texture* tankthreads;
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

vector<LinkedList> redHealthBars = {};
vector<LinkedList> blueHealthBars = {};
vector<SDL_Point> fdiofisdiof = {};

#define LOAD_TEX(_FIELD_) SDL_CreateTextureFromSurface(screen, _FIELD_);

mutex mtx;


// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init()
{
    //initiate grid to allocate memory
    auto instance = Grid::Instance();

    tankthreads = SDL_CreateTexture(screen, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCRWIDTH, SCRHEIGHT);

    background = LOAD_TEX(background_img);
    tank_red = LOAD_TEX(tank_red_img);
    tank_blue = LOAD_TEX(tank_blue_img);
    rocket_red = LOAD_TEX(rocket_red_img);
    rocket_blue = LOAD_TEX(rocket_blue_img);
    smoke = LOAD_TEX(smoke_img);
    explosion = LOAD_TEX(explosion_img);
    particle_beam_sprite = LOAD_TEX(particle_beam_img);

    FPS = TTF_OpenFont("assets/digital-7.ttf", 32); //this opens a font style and sets a size
    End = TTF_OpenFont("assets/digital-7.ttf", 250); //this opens a font style and sets a size

    Uint32* pixels = nullptr;
    int pitch = 0;
    // Now let's make our "pixels" pointer point to the texture data.
    SDL_LockTexture(tankthreads, nullptr, (void**)&pixels, &pitch);
    memcpy(pixels, background_img->pixels, SCRWIDTH * SCRHEIGHT * 4);
    SDL_UnlockTexture(tankthreads);

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
        tanks.push_back(Tank(start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing), BLUE,
                             tank_blue, smoke, 1200, 600, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED));
    }
    //Spawn red tanks
    for (int i = 0; i < NUM_TANKS_RED; i++)
    {
        tanks.push_back(
            Tank(start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing), RED, tank_red,
                 smoke, 80, 80, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED));
    }

    particle_beams.push_back(Particle_beam(vec2<>(SCRWIDTH / 2, SCRHEIGHT / 2), vec2<>(100, 50), particle_beam_sprite,
                                           PARTICLE_BEAM_HIT_VALUE));
    particle_beams.push_back(
        Particle_beam(vec2<>(80, 80), vec2<>(100, 50), particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
    particle_beams.push_back(
        Particle_beam(vec2<>(1200, 600), vec2<>(100, 50), particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));

    for (auto& tank : tanks)
    {
        instance->AddTankToGridCell(&tank);
        if (tank.allignment == RED) redTanks.emplace_back(&tank);
        else blueTanks.emplace_back(&tank);
    }
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown() {}

Game::~Game()
{
    //delete frame_count_font;
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
}

// -----------------------------------------------------------
// Iterates through all tanks and returns the closest enemy tank for the given tank
// -----------------------------------------------------------
Tank& Game::FindClosestEnemy(Tank& current_tank)
{
#if PROFILE_PARALLEL == 1
    EASY_FUNCTION(profiler::colors::Orange);
#endif
    float closest_distance = numeric_limits<float>::infinity();
    int closest_index = 0;

    for (int i = 0; i < tanks.size(); i++)
    {
        if (tanks.at(i).allignment != current_tank.allignment && tanks.at(i).active)
        {
            float sqrDist = fabsf((tanks.at(i).Get_Position() - current_tank.Get_Position()).sqrLength());
            if (sqrDist < closest_distance)
            {
                closest_distance = sqrDist;
                closest_index = i;
            }
        }
    }
    return tanks.at(closest_index);
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
    //Update tanks
    UpdateTanks();

    //Update particle beams
    UpdateParticleBeams();

    //Update smoke plumes
    UpdateSmoke();

    //Update explosion sprites
    UpdateExplosions();

    //Remove when done with remove erase idiom
    explosions.erase(std::remove_if(explosions.begin(), explosions.end(),
                                    [](const Explosion& explosion) { return explosion.done(); }),
                     explosions.end());

    //Update rockets
    UpdateRockets();

    //Remove exploded rockets with remove erase idiom
    rockets.erase(
        std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& rocket) { return !rocket.active; }),
        rockets.end());

    // Sort HP bars
    SortHealthBars();
}

void Game::UpdateTanks()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    tbb::parallel_for(tbb::blocked_range<int>(0, tanks.size()),
                      [&](tbb::blocked_range<int> r)
                      {
#if PROFILE_PARALLEL == 1
                          EASY_BLOCK("Update Tank", profiler::colors::Gold);
#endif
                          for (int i = r.begin(); i < r.end(); ++i)
                          {
                              Tank& tank = tanks[i];
                              if (!tank.active) continue;

                              //Check tank collision and nudge tanks away from each other
                              for (auto cell : Grid::Instance()->GetNeighbouringCells())
                              {
                                  int x = tank.gridCell.x + cell.x;
                                  int y = tank.gridCell.y + cell.y;
                                  if (x < 0 || y < 0 || x > GRID_SIZE_X || y > GRID_SIZE_Y) continue;

                                  for (auto& oTank : Grid::Instance()->grid[x][y])
                                  {
                                      if (&tank == oTank) continue;

                                      vec2<> dir = tank.Get_Position() - oTank->Get_Position();

                                      float colSquaredLen =
                                          (tank.Get_collision_radius() * tank.Get_collision_radius()) +
                                          (oTank->Get_collision_radius() * oTank->Get_collision_radius());

                                      if (dir.sqrLength() < colSquaredLen) { tank.Push(dir.normalized(), 1.f); }
                                  }
                              }

                              //Check if inside particle beam
                              for (Particle_beam& particle_beam : particle_beams)
                              {
                                  if (particle_beam.rectangle.intersectsCircle(tank.Get_Position(),
                                                                               tank.Get_collision_radius())) { if (tank.hit(particle_beam.damage)) { smokes.emplace_back(smoke, tank.position - vec2<>(0, 48)); } }
                              }

                              //Move tanks according to speed and nudges (see above) also reload
                              tank.Tick();

                              //Shoot at closest target if reloaded
                              if (!tank.Rocket_Reloaded()) continue;
                              Tank& target = FindClosestEnemy(tank);
                              scoped_lock lock(mtx);

                              rockets.emplace_back(tank.position,
                                                   (target.Get_Position() - tank.position).normalized() * 3,
                                                   rocket_radius,
                                                   tank.allignment,
                                                   ((tank.allignment == RED) ? rocket_red : rocket_blue));

                              tank.Reload_Rocket();
                          }
                      });
}

void Game::UpdateSmoke()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    for (Smoke& uSmoke : smokes) { uSmoke.Tick(); }
}

void Game::UpdateRockets()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    tbb::parallel_for(tbb::blocked_range<int>(0, rockets.size()),
                      [&](tbb::blocked_range<int> r)
                      {
#if PROFILE_PARALLEL == 1
                          EASY_BLOCK("Update Rocket", profiler::colors::Gold);
#endif
                          for (int i = r.begin(); i < r.end(); ++i)
                          {
                              Rocket& uRocket = rockets[i];
                              uRocket.Tick();

                              //Check if rocket collides with enemy tank, spawn explosion and if tank is destroyed spawn a smoke plume
                              for (auto cell : Grid::Instance()->GetNeighbouringCells())
                              {
                                  vec2<int> rocketGridCell = Grid::GetGridCell(uRocket.position);
                                  int x = rocketGridCell.x + cell.x;
                                  int y = rocketGridCell.y + cell.y;
                                  if (x < 0 || y < 0 || x > GRID_SIZE_X || y > GRID_SIZE_Y) continue;

                                  for (auto& tank : Grid::Instance()->grid[x][y])
                                  {
                                      if (tank->active && (tank->allignment != uRocket.allignment) &&
                                          uRocket.Intersects(tank->position, tank->collision_radius))
                                      {
                                          scoped_lock lock(mtx);
                                          explosions.push_back(Explosion(explosion, tank->position));

                                          if (tank->hit(ROCKET_HIT_VALUE)) { smokes.push_back(Smoke(smoke, tank->position - vec2<>(0, 48))); }

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
    for (Particle_beam& particle_beam : particle_beams) { particle_beam.tick(); }
}

void Game::UpdateExplosions()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    for (Explosion& explosion : explosions) { explosion.Tick(); }
}

void Game::SortHealthBars()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif
    tbb::task_group g;
    g.run([&] { redHealthBars = Sort(redTanks, 100); });
    g.run([&] { blueHealthBars = Sort(blueTanks, 100); });
    g.wait();
}

void Game::Draw()
{
#ifdef USING_EASY_PROFILER
    EASY_FUNCTION(profiler::colors::Yellow);
#endif

    //Draw background
    //SDL_RenderCopy(screen, background, NULL, NULL);
    SDL_RenderCopy(screen, tankthreads, NULL, NULL);

#ifdef USING_EASY_PROFILER
    EASY_BLOCK("Draw tanks", profiler::colors::Red);
#endif
    Uint32* pixels = nullptr;
    int pitch = 0;
    // Now let's make our "pixels" pointer point to the texture data.
    SDL_LockTexture(tankthreads, nullptr, (void**)&pixels, &pitch);

    try
    {
        //Draw sprites
        for (int i = 0; i < NUM_TANKS_BLUE + NUM_TANKS_RED; i++)
        {
            tanks.at(i).Draw(screen);

            vec2 tPos = tanks.at(i).Get_Position();
            // tread marks
            if ((tPos.x >= 0) && (tPos.x < SCRWIDTH) && (tPos.y >= 0) && (tPos.y < SCRHEIGHT))
            {
                // Before setting the color, we need to know where we have to place it.
                Uint32 pixelPosition = (int)tPos.y * (pitch / sizeof(unsigned int)) + (int)tPos.x;
                // Now we can set the pixel(s) we want.
                pixels[pixelPosition] *= 0x808080; //Black;
            }
        }
    }
    catch (...) {}

    // Also don't forget to unlock your texture once you're done.

#ifdef USING_EASY_PROFILER
    EASY_END_BLOCK
#endif

    for (Rocket& r : rockets) { r.Draw(screen); }

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
    for (auto& bucket : redHealthBars)
    {
        Node* currentRedTank = bucket.head;
        while (currentRedTank != nullptr)
        {
            DrawTankHP(countRed, 'r', currentRedTank->value);
            currentRedTank = currentRedTank->next;
            countRed++;
        }
    }

    if (fdiofisdiof.size() > 0) SDL_RenderDrawLines(screen, &fdiofisdiof[0], fdiofisdiof.size());
    fdiofisdiof.clear();

#ifdef USING_EASY_PROFILER
    EASY_END_BLOCK
    EASY_BLOCK("Draw Health_Bar_Blue", profiler::colors::Blue);
#endif
    //Draw sorted health bars blue tanks
    int countBlue = 0;
    for (auto& bucket : blueHealthBars)
    {
        Node* currentBlueTank = bucket.head;
        while (currentBlueTank != nullptr)
        {
            DrawTankHP(countBlue, 'b', currentBlueTank->value);
            currentBlueTank = currentBlueTank->next;
            countBlue++;
        }
    }
    if (fdiofisdiof.size() > 0) SDL_RenderDrawLines(screen, &fdiofisdiof[0], fdiofisdiof.size());
    fdiofisdiof.clear();
#ifdef USING_EASY_PROFILER
    EASY_END_BLOCK
#endif
}

void Game::DrawTankHP(int i, char color, int health)
{
    if (health == TANK_MAX_HEALTH) return;

    int health_bar_start_x = i * (HEALTH_BAR_WIDTH + HEALTH_BAR_SPACING) + HEALTH_BARS_OFFSET_X;
    int health_bar_start_y = (color == 'b') ? 0 : (SCRHEIGHT - HEALTH_BAR_HEIGHT) - 1;
    int health_bar_end_x = health_bar_start_x + HEALTH_BAR_WIDTH;
    int health_bar_end_y = (color == 'b') ? HEALTH_BAR_HEIGHT : SCRHEIGHT - 1;

    fdiofisdiof.emplace_back(SDL_Point{health_bar_start_x, health_bar_start_y});
    fdiofisdiof.emplace_back(
        SDL_Point{health_bar_end_x, health_bar_start_y + (int)((double)HEALTH_BAR_HEIGHT * (1 - ((double)health /
                                                                                                 (double)TANK_MAX_HEALTH)))});
}

// -----------------------------------------------------------
// Sort tanks by health value using bucket sort
// -----------------------------------------------------------
vector<LinkedList> Game::Sort(vector<Tank*>& input, int n_buckets)
{
    vector<LinkedList> buckets(n_buckets);
    for (auto& tank : input) { buckets.at(tank->health / n_buckets).InsertValue(tank->health); }
    return buckets;
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
            duration = perf_timer.elapsed();
            cout << "Duration was: " << duration << " (Replace REF_PERFORMANCE with this value)" << endl;
            lock_update = true;
        }

        frame_count--;
    }

    if (lock_update)
    {
        SDL_Rect r = {420, 170, 450, 260};
        SDL_Rect final_message_rect = {500, 210, 300, 60}; //create a rect

        SDL_SetRenderDrawColor(screen, 0, 0, 0, 255);
        SDL_RenderFillRect(screen, &r);

        int ms = (int)duration % 1000, sec = ((int)duration / 1000) % 60, min = ((int)duration / 60000);
        sprintf(buffer, " %02i:%02i:%03i ", min, sec, ms);
        text_surface = TTF_RenderText_Solid(End, buffer, White);
        text_texture = SDL_CreateTextureFromSurface(screen, text_surface);
        SDL_RenderCopy(screen, text_texture, NULL, &final_message_rect);

        final_message_rect.y = 300;

        sprintf(buffer, "SPEEDUP: %4.1f", REF_PERFORMANCE / duration);
        text_surface = TTF_RenderText_Solid(End, buffer, White);
        text_texture = SDL_CreateTextureFromSurface(screen, text_surface);
        SDL_RenderCopy(screen, text_texture, NULL, &final_message_rect);
    }
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick(float deltaTime)
{
    tbb::task_group g;
    if (!lock_update)
    {
        g.run([&]
        {
#ifdef USING_EASY_PROFILER
            EASY_BLOCK("SDL_UnlockTexture", profiler::colors::Orange);
#endif
            SDL_UnlockTexture(tankthreads);
        });
        Update(deltaTime);
    }
    else { SDL_UnlockTexture(tankthreads); }

    Draw();

    MeasurePerformance();

#if PROFILE_PARALLEL == 1
    EASY_BLOCK("Print frame count", profiler::colors::Gold);
#endif
    if (!lock_update)
    {
        //Print frame count
        frame_count++;
        char buffer[15];
        sprintf(buffer, "FRAME: %lld", frame_count);
        text_surface = TTF_RenderText_Solid(FPS, buffer, White);
        text_texture = SDL_CreateTextureFromSurface(screen, text_surface);

        SDL_RenderCopy(screen, text_texture, NULL, &framecounter_message_rect);
        g.wait();
    }
}
