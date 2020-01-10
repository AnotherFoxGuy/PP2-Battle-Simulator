#include "rocket.h"

#define R_SIZE 25

namespace PP2
{
Rocket::Rocket(vec2<> position, vec2<> direction, float collision_radius, allignments allignment, SDL_Texture* rocket_sprite)
    : position(position), speed(direction), collision_radius(collision_radius), allignment(allignment),
      current_frame(0), rocket_sprite(rocket_sprite), active(true), id(rand())
{
    SrcR = {0, 0, R_SIZE, R_SIZE};
    DestR = {0, 0, R_SIZE, R_SIZE};
}

Rocket::~Rocket() { }

void Rocket::Tick()
{
    position += speed;
    if (++current_frame > 8) current_frame = 0;
}

//Draw the sprite with the facing based on this rockets movement direction
void Rocket::Draw(SDL_Renderer* screen)
{
    /*rocket_sprite->SetFrame(((abs(speed.x) > abs(speed.y)) ? ((speed.x < 0) ? 3 : 0) : ((speed.y < 0) ? 9 : 6)) +
                            (current_frame / 3));
    rocket_sprite->Draw(screen, (int) position.x - 12, (int) position.y - 12);*/
    DestR.x = (int)position.x - 12;
    DestR.y = (int)position.y - 12;

    int frame = ((abs(speed.x) > abs(speed.y)) ? ((speed.x < 0) ? 3 : 0) : ((speed.y < 0) ? 9 : 6)) +
                (current_frame / 3);

    SrcR.x = frame * R_SIZE;

    SDL_RenderCopy(screen, rocket_sprite, &SrcR, &DestR);
}

//Does the given circle collide with this rockets collision circle?
bool Rocket::Intersects(vec2<> position_other, float radius_other) const
{
    //Uses squared lengths to remove expensive square roots
    float distance_sqr = (position_other - position).sqrLength();

    return distance_sqr <= ((collision_radius * collision_radius) + (radius_other * radius_other));
}
} // namespace PP2
