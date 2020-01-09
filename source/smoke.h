#pragma once

#include "surface.h"
#include "template.h"

namespace PP2 {

    class Smoke {
    public:
        Smoke(SDL_Texture *smoke_sprite, vec2<> position) : current_frame(0), smoke_sprite(smoke_sprite), position(position) {
            SrcR.x = 0;
            SrcR.y = 0;
            SrcR.w = 14;
            SrcR.h = 18;
        }

        void Tick();

        void Draw(SDL_Renderer *screen);

        vec2<> position;

        int current_frame;
        SDL_Texture *smoke_sprite;
        SDL_Rect SrcR;
        SDL_Rect DestR;
    };
} // namespace PP2