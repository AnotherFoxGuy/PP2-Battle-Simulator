
#pragma once

#define GETIMAGE(X,Y,FRAME) \
    SDL_Rect{X, Y * FRAME, X, Y};