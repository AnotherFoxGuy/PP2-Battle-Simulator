#pragma once

//Global performance timer
// base speeds
// Roland:  69931
// Sietze:  65732
// Gert:    73804
#define REF_PERFORMANCE 65732 //UPDATE THIS WITH YOUR REFERENCE PERFORMANCE (see console after 2k frames)

#define UINT16 uint16_t

#define SCRWIDTH 1280
#define SCRHEIGHT 720

#define NUM_TANKS_BLUE 1279
#define NUM_TANKS_RED 1279

#define GRID_SIZE_X 24
#define GRID_SIZE_Y 32

#define TANK_MAX_HEALTH 1000
#define ROCKET_HIT_VALUE 60
#define PARTICLE_BEAM_HIT_VALUE 50

#define TANK_MAX_SPEED 1.5

#define HEALTH_BARS_OFFSET_X 0
#define HEALTH_BAR_HEIGHT 70
#define HEALTH_BAR_WIDTH 1
#define HEALTH_BAR_SPACING 0

#define MAX_FRAMES 2000

#define MAX_TANKS NUM_TANKS_BLUE + NUM_TANKS_RED