using namespace std;

#include "surface.h"
#include "template.h"
#include <cstdio>
#include <iostream>
#include <string>

using namespace PP2;

#include <tbb/parallel_for.h>
#include <tbb/task_group.h>
#include "ThreadPool.h"
#include "Grid.h"
#include "explosion.h"
#include "particle_beam.h"
#include "rocket.h"
#include "smoke.h"
#include "tank.h"

#include "game.h"

#ifdef USE_MICROPROFILE
#include "microprofile.h"
#endif

static timer perf_timer;
static float duration;

//Load sprite files and initialize sprites
static Surface *background_img = new Surface("assets/Background_Grass.png");
static Surface *tank_red_img = new Surface("assets/Tank_Proj2.png");
static Surface *tank_blue_img = new Surface("assets/Tank_Blue_Proj2.png");
static Surface *rocket_red_img = new Surface("assets/Rocket_Proj2.png");
static Surface *rocket_blue_img = new Surface("assets/Rocket_Blue_Proj2.png");
static Surface *particle_beam_img = new Surface("assets/Particle_Beam.png");
static Surface *smoke_img = new Surface("assets/Smoke.png");
static Surface *explosion_img = new Surface("assets/Explosion.png");

static Sprite background(background_img, 1);
static Sprite tank_red(tank_red_img, 12);
static Sprite tank_blue(tank_blue_img, 12);
static Sprite rocket_red(rocket_red_img, 12);
static Sprite rocket_blue(rocket_blue_img, 12);
static Sprite smoke(smoke_img, 4);
static Sprite explosion(explosion_img, 9);
static Sprite particle_beam_sprite(particle_beam_img, 3);

const static vec2<> tank_size(14, 18);
const static vec2<> rocket_size(25, 24);

const static float tank_radius = 12.f;
const static float rocket_radius = 10.f;

const static int threadCount = std::thread::hardware_concurrency() * 2;
mutex mtx;

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init() {
    //initiate grid to allocate memory
    auto instance = Grid::Instance();

    frame_count_font = new Font("assets/digital_small.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");

    tanks.reserve(NUM_TANKS_BLUE + NUM_TANKS_RED);
    blueTanks.reserve(NUM_TANKS_BLUE);
    redTanks.reserve(NUM_TANKS_RED);

    uint rows = (uint) sqrt(NUM_TANKS_BLUE + NUM_TANKS_RED);
    uint max_rows = 12;

    float start_blue_x = tank_size.x + 10.0f;
    float start_blue_y = tank_size.y + 80.0f;

    float start_red_x = 980.0f;
    float start_red_y = 100.0f;

    float spacing = 15.0f;

    //Spawn blue tanks
    for (int i = 0; i < NUM_TANKS_BLUE; i++) {
        tanks.push_back(Tank(start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing), BLUE,
                             &tank_blue, &smoke, 1200, 600, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED));
    }
    //Spawn red tanks
    for (int i = 0; i < NUM_TANKS_RED; i++) {
        tanks.push_back(
                Tank(start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing), RED, &tank_red,
                     &smoke, 80, 80, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED));
    }

    particle_beams.push_back(Particle_beam(vec2<>(SCRWIDTH / 2, SCRHEIGHT / 2), vec2<>(100, 50), &particle_beam_sprite,
                                           PARTICLE_BEAM_HIT_VALUE));
    particle_beams.push_back(
            Particle_beam(vec2<>(80, 80), vec2<>(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
    particle_beams.push_back(
            Particle_beam(vec2<>(1200, 600), vec2<>(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));

    for (auto &tank : tanks) {
        instance->AddTankToGridCell(&tank);
        if (tank.allignment == RED)
            redTanks.emplace_back(&tank);
        else
            blueTanks.emplace_back(&tank);
    }

}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown() {
}

Game::~Game() {
    delete frame_count_font;
}

// -----------------------------------------------------------
// Iterates through all tanks and returns the closest enemy tank for the given tank
// -----------------------------------------------------------
Tank &Game::FindClosestEnemy(Tank &current_tank) {
    float closest_distance = numeric_limits<float>::infinity();
    int closest_index = 0;

    for (int i = 0; i < tanks.size(); i++) {
        if (tanks.at(i).allignment != current_tank.allignment && tanks.at(i).active) {
            float sqrDist = fabsf((tanks.at(i).Get_Position() - current_tank.Get_Position()).sqrLength());
            if (sqrDist < closest_distance) {
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
void Game::Update(float deltaTime) {
#ifdef USE_MICROPROFILE
    MICROPROFILE_SCOPEI("Game", "Update", MP_YELLOW);
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
                                    [](const Explosion &explosion) { return explosion.done(); }),
                     explosions.end());

    //Update rockets
    UpdateRockets();

    //Remove exploded rockets with remove erase idiom
    rockets.erase(
            std::remove_if(rockets.begin(), rockets.end(), [](const Rocket &rocket) { return !rocket.active; }),
            rockets.end());
}

void Game::UpdateTanks() {
#ifdef USE_MICROPROFILE
    MICROPROFILE_SCOPEI("Game", "UpdateTanks", MP_YELLOW);
#endif
    tbb::parallel_for(tbb::blocked_range<int>(1, tanks.size()),
                      [&](tbb::blocked_range<int> r) {
                          for (int i = r.begin(); i < r.end(); ++i) {
                              Tank &tank = tanks[i];
                              if (!tank.active)
                                  continue;

                              //Check tank collision and nudge tanks away from each other
                              for (auto cell : Grid::Instance()->GetNeighbouringCells()) {
                                  int x = tank.gridCell.x + cell.x;
                                  int y = tank.gridCell.y + cell.y;
                                  if (x < 0 || y < 0 || x > GRID_SIZE_X || y > GRID_SIZE_Y)
                                      continue;

                                  for (auto &oTank : Grid::Instance()->grid[x][y]) {
                                      if (&tank == oTank) continue;

                                      vec2<> dir = tank.Get_Position() - oTank->Get_Position();

                                      float colSquaredLen =
                                              (tank.Get_collision_radius() * tank.Get_collision_radius()) +
                                              (oTank->Get_collision_radius() * oTank->Get_collision_radius());

                                      if (dir.sqrLength() < colSquaredLen) {
                                          tank.Push(dir.normalized(), 1.f);
                                      }
                                  }
                              }

                              //Check if inside particle beam
                              for (Particle_beam &particle_beam : particle_beams) {
                                  if (particle_beam.rectangle.intersectsCircle(tank.Get_Position(),
                                                                               tank.Get_collision_radius())) {
                                      if (tank.hit(particle_beam.damage)) {
                                          smokes.emplace_back(smoke, tank.position - vec2<>(0, 48));
                                      }
                                  }
                              }

                              //Move tanks according to speed and nudges (see above) also reload
                              tank.Tick();

                              //Shoot at closest target if reloaded
                              if (!tank.Rocket_Reloaded())
                                  continue;
                              Tank &target = FindClosestEnemy(tank);
                              scoped_lock lock(mtx);

                              rockets.emplace_back(tank.position,
                                                   (target.Get_Position() - tank.position).normalized() * 3,
                                                   rocket_radius,
                                                   tank.allignment,
                                                   ((tank.allignment == RED) ? &rocket_red : &rocket_blue));

                              tank.Reload_Rocket();
                          }
                      });
}

void Game::UpdateSmoke() {
#ifdef USE_MICROPROFILE
    MICROPROFILE_SCOPEI("Game", "UpdateSmoke", MP_YELLOW);
#endif
    for (Smoke &uSmoke : smokes) {
        uSmoke.Tick();
    }
}

void Game::UpdateRockets() {
#ifdef USE_MICROPROFILE
    MICROPROFILE_SCOPEI("Game", "UpdateRockets", MP_YELLOW);
#endif
    tbb::parallel_for(tbb::blocked_range<int>(1, rockets.size()),
                      [&](tbb::blocked_range<int> r) {
                          for (int i = r.begin(); i < r.end(); ++i) {
                              Rocket &uRocket = rockets[i];
                              uRocket.Tick();

                              //Check if rocket collides with enemy tank, spawn explosion and if tank is destroyed spawn a smoke plume
                              for (auto cell : Grid::Instance()->GetNeighbouringCells()) {
                                  vec2<int> rocketGridCell = Grid::GetGridCell(uRocket.position);
                                  int x = rocketGridCell.x + cell.x;
                                  int y = rocketGridCell.y + cell.y;
                                  if (x < 0 || y < 0 || x > GRID_SIZE_X || y > GRID_SIZE_Y)
                                      continue;

                                  for (auto &tank : Grid::Instance()->grid[x][y]) {
                                      if (tank->active && (tank->allignment != uRocket.allignment) &&
                                          uRocket.Intersects(tank->position, tank->collision_radius)) {
                                          scoped_lock lock(mtx);
                                          explosions.push_back(Explosion(&explosion, tank->position));

                                          if (tank->hit(ROCKET_HIT_VALUE)) {
                                              smokes.push_back(Smoke(smoke, tank->position - vec2<>(0, 48)));
                                          }

                                          uRocket.active = false;
                                          break;
                                      }
                                  }
                              }
                          }
                      });
}

void Game::UpdateParticleBeams() {
    for (Particle_beam &particle_beam : particle_beams) {
        particle_beam.tick();
    }
}

void Game::UpdateExplosions() {
#ifdef USE_MICROPROFILE
    MICROPROFILE_SCOPEI("Game", "UpdateExplosions", MP_YELLOW);
#endif
    for (Explosion &explosion : explosions) {
        explosion.Tick();
    }
}

void Game::Draw() {
    vector <future<void>> tasks = {};

    // clear the graphics window
    screen->Clear(0x00);

    //Draw background
    background.Draw(screen, 0, 0);

    tbb::parallel_for(tbb::blocked_range<int>(1, smokes.size()),
                      [&](tbb::blocked_range<int> r) {
                          for (int i = r.begin(); i < r.end(); ++i) {
                              smokes[i].Draw(screen);
                          }
                      });


    tbb::parallel_for(tbb::blocked_range<int>(1, NUM_TANKS_BLUE + NUM_TANKS_RED),
                      [&](tbb::blocked_range<int> r) {
                          for (int i = r.begin(); i < r.end(); ++i) {
                              tanks.at(i).Draw(screen);

                              vec2<> tPos = tanks.at(i).Get_Position();
                              // tread marks
                              if ((tPos.x >= 0) && (tPos.x < SCRWIDTH) && (tPos.y >= 0) && (tPos.y < SCRHEIGHT))
                                  background.GetBuffer()[(int) tPos.x + (int) tPos.y * SCRWIDTH] = SubBlend(
                                          background.GetBuffer()[(int) tPos.x + (int) tPos.y * SCRWIDTH], 0x808080);
                          }
                      });

    tbb::parallel_for(tbb::blocked_range<int>(1, rockets.size()),
                      [&](tbb::blocked_range<int> r) {
                          for (int i = r.begin(); i < r.end(); ++i) {
                              rockets[i].Draw(screen);
                          }
                      });


    tbb::parallel_for(tbb::blocked_range<int>(1, explosions.size()),
                      [&](tbb::blocked_range<int> r) {
                          for (int i = r.begin(); i < r.end(); ++i) {
                              explosions[i].Draw(screen);
                          }
                      });

    tbb::parallel_for(tbb::blocked_range<int>(1, particle_beams.size()),
                      [&](tbb::blocked_range<int> r) {
                          for (int i = r.begin(); i < r.end(); ++i) {
                              particle_beams[i].Draw(screen);
                          }
                      });

    tbb::task_group group;

    group.run([&] {
        //Draw sorted health bars red tanks
        vector <LinkedList> redHealthBars = Sort(redTanks, 100);
        int countRed = 0;
        for (auto &bucket : redHealthBars) {
            Node *currentRedTank = bucket.head;
            while (currentRedTank != nullptr) {
                DrawTankHP(countRed, 'r', currentRedTank->value);
                currentRedTank = currentRedTank->next;
                countRed++;
            }
        }
    });

    group.run([&] {
        //Draw sorted health bars blue tanks
        vector <LinkedList> blueHealthBars = Sort(blueTanks, 100);
        int countBlue = 0;
        for (auto &bucket : blueHealthBars) {
            Node *currentBlueTank = bucket.head;
            while (currentBlueTank != nullptr) {
                DrawTankHP(countBlue, 'b', currentBlueTank->value);
                currentBlueTank = currentBlueTank->next;
                countBlue++;
            }
        }
    });
    group.wait();
}

void Game::DrawTankHP(int i, char color, int health) {
    int health_bar_start_x = i * (HEALTH_BAR_WIDTH + HEALTH_BAR_SPACING) + HEALTH_BARS_OFFSET_X;
    int health_bar_start_y = (color == 'b') ? 0 : (SCRHEIGHT - HEALTH_BAR_HEIGHT) - 1;
    int health_bar_end_x = health_bar_start_x + HEALTH_BAR_WIDTH;
    int health_bar_end_y = (color == 'b') ? HEALTH_BAR_HEIGHT : SCRHEIGHT - 1;

    screen->Bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
    screen->Bar(health_bar_start_x, health_bar_start_y + (int) ((double) HEALTH_BAR_HEIGHT *
                                                                (1 - ((double) health / (double) TANK_MAX_HEALTH))),
                health_bar_end_x, health_bar_end_y, GREENMASK);
}

// -----------------------------------------------------------
// Sort tanks by health value using bucket sort
// -----------------------------------------------------------
vector<LinkedList> Game::Sort(vector<Tank *> &input, int n_buckets) {
    vector <LinkedList> buckets(n_buckets);
    for (auto &tank : input) {
        buckets.at(tank->health / n_buckets).InsertValue(tank->health);
    }
    return buckets;
}

// -----------------------------------------------------------
// When we reach MAX_FRAMES print the duration and speedup multiplier
// Updating REF_PERFORMANCE at the top of this file with the value
// on your machine gives you an idea of the speedup your optimizations give
// -----------------------------------------------------------
void PP2::Game::MeasurePerformance() {
    char buffer[128];
    if (frame_count >= MAX_FRAMES) {
        if (!lock_update) {
            duration = perf_timer.elapsed();
            cout << "Duration was: " << duration << " (Replace REF_PERFORMANCE with this value)" << endl;
            lock_update = true;
        }

        frame_count--;
    }

    if (lock_update) {
        screen->Bar(420, 170, 870, 430, 0x030000);
        int ms = (int) duration % 1000, sec = ((int) duration / 1000) % 60, min = ((int) duration / 60000);
        sprintf(buffer, "%02i:%02i:%03i", min, sec, ms);
        frame_count_font->Centre(screen, buffer, 200);
        sprintf(buffer, "SPEEDUP: %4.1f", REF_PERFORMANCE / duration);
        frame_count_font->Centre(screen, buffer, 340);
    }
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick(float deltaTime) {
    if (!lock_update) {
        Update(deltaTime);
    }
    Draw();

    MeasurePerformance();

    // print something in the graphics window
    //screen->Print("hello world", 2, 2, 0xffffff);

    // print something to the text window
    //cout << "This goes to the console window." << std::endl;

    //Print frame count
    frame_count++;
    string frame_count_string = "FRAME: " + std::to_string(frame_count);
    frame_count_font->Print(screen, frame_count_string.c_str(), 350, 580);
}

