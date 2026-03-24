#ifndef FD2_PALETTE_H
#define FD2_PALETTE_H

#include "fd2_types.h"

void fd2_apply_palette_6bit(byte* palette_6bit, byte palette[256][3]);
void fd2_load_palette_data(byte* palette_6bit, byte palette[256][3]);
void fd2_set_palette_start_end(byte palette[256][3], int start, int end);
void fd2_fade_palette(byte src[256][3], byte dst[256][3], byte palette_out[256][3], int steps, int current_step);
void fd2_set_brightness(byte palette_in[256][3], byte palette_out[256][3], int brightness);

#ifdef USE_SDL
void fd2_sdl_apply_palette(byte palette[256][3]);
#endif

#endif
