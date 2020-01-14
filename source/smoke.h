#pragma once

#include <SDL2/SDL_render.h>
#include "template.h"

namespace PP2
{
class Smoke
{
public:
    Smoke(SDL_Texture* smoke_sprite, vec2<> position);

    void Tick();

    void Draw(SDL_Renderer* screen);

    vec2<> position;

    int current_frame;
    SDL_Texture* smoke_sprite;
    SDL_Rect SrcR;
    SDL_Rect DestR;
};
} // namespace PP2
