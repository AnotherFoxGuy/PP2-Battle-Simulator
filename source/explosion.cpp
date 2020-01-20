#include "explosion.h"
#define E_SIZE 32

PP2::Explosion::Explosion(SDL_Texture* explosion_sprite, vec2<> position)
    : current_frame(0), explosion_sprite(explosion_sprite),
      position(position)
{
    SrcR = {0, 0, E_SIZE, E_SIZE};
    DestR = {0, 0, E_SIZE, E_SIZE};
}

bool PP2::Explosion::done() const { return current_frame > 17; }

void PP2::Explosion::Tick() { if (current_frame < 18) current_frame++; }

void PP2::Explosion::Draw(SDL_Renderer* screen)
{
    if(!InScreen(position))
        return;

    /* explosion_sprite->SetFrame(current_frame / 2);
     explosion_sprite->Draw(screen, (int) position.x, (int) position.y);*/
    DestR.x = (int)position.x - 16;
    DestR.y = (int)position.y - 16;

    SrcR.x = (current_frame / 2) * E_SIZE;

    SDL_RenderCopy(screen, explosion_sprite, &SrcR, &DestR);
}
