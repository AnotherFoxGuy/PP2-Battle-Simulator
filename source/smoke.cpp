#include "smoke.h"

#define S_SIZE 32

namespace PP2
{
Smoke::Smoke(SDL_Texture* smoke_sprite, vec2<> position)
    : current_frame(0), smoke_sprite(smoke_sprite), position(position)
{
    SrcR = {0, 0, S_SIZE, S_SIZE};
    DestR = {0, 0, S_SIZE, S_SIZE};
}

void Smoke::Tick() { if (++current_frame == 60) current_frame = 0; }

void Smoke::Draw(SDL_Renderer* screen)
{
    /*smoke_sprite.SetFrame(current_frame / 15);

    smoke_sprite.Draw(screen, (int) position.x, (int) position.y);*/

    DestR.x = (int)position.x;
    DestR.y = (int)position.y;

    SrcR.x = (current_frame / 15) * S_SIZE;

    SDL_RenderCopy(screen, smoke_sprite, &SrcR, &DestR);
}
} // namespace PP2
