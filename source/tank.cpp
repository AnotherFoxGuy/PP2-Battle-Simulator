#include "tank.h"
#include "Grid.h"
#include <SDL.h>
#include <iostream>

#define T_SIZE_X 14
#define T_SIZE_Y 18

namespace PP2
{
Tank::Tank(
    float pos_x,
    float pos_y,
    allignments allignment,
    SDL_Texture* tank_sprite,
    SDL_Texture* smoke_sprite,
    float tar_x,
    float tar_y,
    float collision_radius,
    int health,
    float max_speed)
    : position(pos_x, pos_y),
      gridCell(Grid::GetGridCell(position)),
      allignment(allignment),
      target(tar_x, tar_y),
      health(health),
      collision_radius(collision_radius),
      max_speed(max_speed),
      force(0, 0),
      reload_time(1),
      reloaded(false),
      speed(0),
      active(true),
      current_frame(0),
      tank_sprite(tank_sprite),
      smoke_sprite(smoke_sprite)
{
    SrcR = {0, 0, T_SIZE_X, T_SIZE_Y};
    DestR = {0, 0, T_SIZE_X, T_SIZE_Y};
}

Tank::~Tank() {}

void Tank::Tick()
{
    vec2<> direction = (target - position).normalized();

    //Update using accumulated force
    speed = direction + force;
    position += speed * max_speed * 0.5f;

    //Update reload time
    if (--reload_time <= 0.0f) { reloaded = true; }

    auto newGridCell = Grid::GetGridCell(position);
    if (gridCell != newGridCell)
    {
        //Move tank to the new grid cell
        Grid::Instance()->MoveTankToGridCell(this, newGridCell);
        //Update grid cell
        gridCell = Grid::GetGridCell(position);
    }

    force = vec2(0.f, 0.f);

    if (++current_frame > 8) current_frame = 0;
}

//Start reloading timer
void Tank::Reload_Rocket() { reloaded = false, reload_time = 200.0f; }

void Tank::Deactivate() { active = false; }

//Remove health
bool Tank::hit(int hit_value)
{
    health -= hit_value;

    if (health <= 0)
    {
        this->Deactivate();
        return true;
    }

    return false;
}

//Draw the sprite with the facing based on this tanks movement direction
void Tank::Draw(SDL_Renderer* screen)
{
    if (!InScreen(position))
        return;

    vec2<> direction = (target - position).normalized();
    /*tank_sprite->SetFrame(
            ((abs(direction.x) > abs(direction.y)) ? ((direction.x < 0) ? 3 : 0) : ((direction.y < 0) ? 9 : 6)) +
            (current_frame / 3));
        tank_sprite->Draw(screen, (int)position.x - 14, (int)position.y - 18);*/
    DestR.x = (int)position.x - 9;
    DestR.y = (int)position.y - 9;

    int frame =
        ((abs(direction.x) > abs(direction.y)) ? ((direction.x < 0) ? 3 : 0) : ((direction.y < 0) ? 9 : 6)) +
        (current_frame / 3);
    SrcR.x = frame * T_SIZE_X;

    SDL_RenderCopy(screen, tank_sprite, &SrcR, &DestR);
}

int Tank::CompareHealth(const Tank& other) const
{
    return ((health == other.health) ? 0 : ((health > other.health) ? 1 : -1));
}

//Add some force in a given direction
void Tank::Push(vec2<> direction, float magnitude) { force += direction * magnitude; }
} // namespace PP2
