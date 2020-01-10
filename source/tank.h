#pragma once

#include "template.h"

namespace PP2
{
enum allignments
{
    BLUE,
    RED
};

class Tank
{
public:
    Tank(float pos_x, float pos_y, allignments allignment, SDL_Texture* tank_sprite, SDL_Texture* smoke_sprite, float tar_x,
         float tar_y, float collision_radius, int health, float max_speed);

    ~Tank();

    void Tick();

    vec2<> Get_Position() const { return position; };

    float Get_collision_radius() const { return collision_radius; };

    bool Rocket_Reloaded() const { return reloaded; };

    void Reload_Rocket();

    void Deactivate();

    bool hit(int hit_value);

    void Draw(SDL_Renderer* screen);

    int CompareHealth(const Tank& other) const;

    void Push(vec2<float> direction, float magnitude);

    vec2<float> position;
    vec2<int> gridCell;
    vec2<float> speed;
    vec2<float> target;

    int health;

    float collision_radius;
    vec2<float> force;

    float max_speed;
    float reload_time;

    bool reloaded;
    bool active;

    allignments allignment;

    int current_frame;
    SDL_Texture* tank_sprite;
    SDL_Texture* smoke_sprite;

    SDL_Rect SrcR;
    SDL_Rect DestR;
};
} // namespace PP2
