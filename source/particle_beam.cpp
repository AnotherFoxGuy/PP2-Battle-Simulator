#include "particle_beam.h"

#define P_SIZE_X 150
#define P_SIZE_Y 200

namespace PP2
{
Particle_beam::Particle_beam()
    : min_position(), max_position(), particle_beam_sprite(nullptr), sprite_frame(0), rectangle(), damage(1)
{
    SrcR = {0, 0, P_SIZE_X, P_SIZE_Y};
    DestR = {0, 0, P_SIZE_X, P_SIZE_Y};
}

Particle_beam::Particle_beam(vec2<> min, vec2<> max, SDL_Texture* particle_beam_sprite, int damage)
    : particle_beam_sprite(
          particle_beam_sprite), sprite_frame(0), damage(damage)
{
    min_position = min;
    max_position = min + max;
    SrcR = {0, 0, P_SIZE_X, P_SIZE_Y};
    DestR = {0, 0, P_SIZE_X, P_SIZE_Y};
    rectangle = Rectangle2D(min_position, max_position);
}

void Particle_beam::tick() { if (++sprite_frame == 30) { sprite_frame = 0; } }

void Particle_beam::Draw(SDL_Renderer* screen)
{
    vec2<> position = rectangle.min;

    const int offsetX = 23;
    const int offsetY = 137;

    /*particle_beam_sprite->SetFrame(sprite_frame / 10);
    particle_beam_sprite->Draw(screen, (int) (position.x - offsetX), (int) (position.y - offsetY));*/

    DestR.x = (int)(position.x - offsetX);
    DestR.y = (int)(position.y - offsetY);

    SrcR.x = (sprite_frame / 10) * P_SIZE_X;

    SDL_RenderCopy(screen, particle_beam_sprite, &SrcR, &DestR);
}
} // namespace PP2
