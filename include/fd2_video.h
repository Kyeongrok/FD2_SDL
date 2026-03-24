#ifndef FD2_VIDEO_H
#define FD2_VIDEO_H

#include "fd2_types.h"

void fd2_vga_set_color(int index, byte r, byte g, byte b);
void fd2_set_palette_range(int start, int end, int brightness_subtract);
void fd2_fade_palette_step(int start, int end, int current_step, int total_steps, int brightness);
void fd2_copy_to_vga(byte* src, int src_stride, byte* dst, int dst_stride, int width, int height);

#ifdef USE_SDL
extern void fd2_sdl_set_palette_6bit(int start, int end, const byte* palette_data);
void fd2_sdl_set_palette_from_6bit(int start, int end, const byte* palette_6bit);
#endif

#endif
