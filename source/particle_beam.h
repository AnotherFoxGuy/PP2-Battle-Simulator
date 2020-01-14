#pragma once

#include <vector>
#include <SDL2/SDL_render.h>
#include "template.h"

namespace PP2
{
class Particle_beam
{
public:
    Particle_beam();

    Particle_beam(vec2<> min, vec2<> max, SDL_Texture* particle_beam_sprite, int damage);

    void tick();

    void Draw(SDL_Renderer* screen);

    vec2<> min_position;
    vec2<> max_position;

    Rectangle2D rectangle;

    int sprite_frame;

    int damage;

    SDL_Texture* particle_beam_sprite;

    SDL_Rect SrcR;
    SDL_Rect DestR;
};
} // namespace PP2
