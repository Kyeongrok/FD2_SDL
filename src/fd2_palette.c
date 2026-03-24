#include "fd2_types.h"
#include "fd2_palette.h"
#include <stdio.h>
#include <string.h>

void fd2_apply_palette_6bit(byte* palette_6bit, byte palette[256][3]) {
    if (!palette_6bit) return;
    for (int i = 0; i < 256; i++) {
        byte r6 = palette_6bit[i * 3] & 0x3F;
        byte g6 = palette_6bit[i * 3 + 1] & 0x3F;
        byte b6 = palette_6bit[i * 3 + 2] & 0x3F;
        palette[i][0] = (r6 << 2) | (r6 >> 4);
        palette[i][1] = (g6 << 2) | (g6 >> 4);
        palette[i][2] = (b6 << 2) | (b6 >> 4);
    }
}

void fd2_load_palette_data(byte* palette_6bit, byte palette[256][3]) {
    if (palette_6bit) {
        fd2_apply_palette_6bit(palette_6bit, palette);
    }
}

void fd2_set_palette_start_end(byte palette[256][3], int start, int end) {
    (void)palette;
    (void)start;
    (void)end;
}

void fd2_fade_palette(byte src[256][3], byte dst[256][3], byte palette_out[256][3], int steps, int current_step) {
    if (current_step >= steps) {
        memcpy(palette_out, dst, sizeof(byte) * 256 * 3);
        return;
    }
    
    int diff_r, diff_g, diff_b;
    for (int i = 0; i < 256; i++) {
        diff_r = (int)dst[i][0] - (int)src[i][0];
        diff_g = (int)dst[i][1] - (int)src[i][1];
        diff_b = (int)dst[i][2] - (int)src[i][2];
        
        palette_out[i][0] = src[i][0] + (diff_r * current_step) / steps;
        palette_out[i][1] = src[i][1] + (diff_g * current_step) / steps;
        palette_out[i][2] = src[i][2] + (diff_b * current_step) / steps;
    }
}

void fd2_set_brightness(byte palette_in[256][3], byte palette_out[256][3], int brightness) {
    for (int i = 0; i < 256; i++) {
        int r = (int)palette_in[i][0] + brightness;
        int g = (int)palette_in[i][1] + brightness;
        int b = (int)palette_in[i][2] + brightness;
        
        if (r < 0) r = 0;
        if (r > 255) r = 255;
        if (g < 0) g = 0;
        if (g > 255) g = 255;
        if (b < 0) b = 0;
        if (b > 255) b = 255;
        
        palette_out[i][0] = (byte)r;
        palette_out[i][1] = (byte)g;
        palette_out[i][2] = (byte)b;
    }
}

#ifdef USE_SDL
extern void fd2_sdl_set_palette_6bit(int start, int end, const byte* palette_data);

void fd2_sdl_apply_palette(byte palette[256][3]) {
    fd2_sdl_set_palette_6bit(0, 255, (const byte*)palette);
}
#endif
