#pragma once

#include "template.h"
#include "tank.h"
#include "surface.h"

namespace PP2
{
class Rocket
{
public:
    Rocket(vec2<> position, vec2<> direction, float collision_radius, allignments allignment, SDL_Texture* rocket_sprite);

    ~Rocket();

    void Tick();

    void Draw(SDL_Renderer* screen);

    bool Intersects(vec2<> position_other, float radius_other) const;

    vec2<> position;
    vec2<> speed;

    int id;

    float collision_radius;

    bool active;

    allignments allignment;

    int current_frame;
    SDL_Texture* rocket_sprite;

    SDL_Rect SrcR;
    SDL_Rect DestR;
};
} // namespace PP2
