#pragma once

#include "surface.h"
#include "template.h"

namespace PP2
{
class Explosion
{
public:
    Explosion(SDL_Texture* explosion_sprite, vec2<> position);

    bool done() const;

    void Tick();

    void Draw(SDL_Renderer* screen);

    vec2<> position;

    int current_frame;
    SDL_Texture* explosion_sprite;
    SDL_Rect SrcR;
    SDL_Rect DestR;
};
}
