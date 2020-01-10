#include "smoke.h"

namespace PP2 {

    void Smoke::Tick() {
        if (++current_frame == 60) current_frame = 0;
    }

    void Smoke::Draw(SDL_Renderer *screen) {
        /*smoke_sprite.SetFrame(current_frame / 15);

        smoke_sprite.Draw(screen, (int) position.x, (int) position.y);*/

        DestR.x = (int) position.x;
        DestR.y = (int) position.y;
        DestR.w = 32;
        DestR.h = 32;

        SDL_RenderCopy(screen, smoke_sprite, &SrcR, &DestR);
    }

} // namespace PP2