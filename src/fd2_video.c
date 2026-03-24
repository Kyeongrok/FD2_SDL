#include "fd2_types.h"
#include "fd2_video.h"
#include <string.h>

void fd2_vga_set_color(int index, byte r, byte g, byte b) {
    (void)index;
    (void)r;
    (void)g;
    (void)b;
}

void fd2_set_palette_range(int start, int end, int brightness_subtract) {
    (void)start;
    (void)end;
    (void)brightness_subtract;
}

void fd2_set_palette_from_data(byte* palette_data, int start, int end, int brightness_subtract) {
    for (int i = start; i <= end; i++) {
        int r = palette_data[3 * i] - brightness_subtract;
        int g = palette_data[3 * i + 1] - brightness_subtract;
        int b = palette_data[3 * i + 2] - brightness_subtract;
        if (r < 0) r = 0;
        if (g < 0) g = 0;
        if (b < 0) b = 0;
        fd2_vga_set_color(i, r, g, b);
    }
}

void fd2_fade_palette_step(int start, int end, int current_step, int total_steps, int brightness) {
    (void)start;
    (void)end;
    (void)current_step;
    (void)total_steps;
    (void)brightness;
}

void fd2_copy_to_vga(byte* src, int src_stride, byte* dst, int dst_stride, int width, int height) {
    if (src_stride == dst_stride) {
        memcpy(dst, src, dst_stride * height);
    } else {
        for (int y = 0; y < height; y++) {
            memcpy(dst + y * dst_stride, src + y * src_stride, width);
        }
    }
}

#ifdef USE_SDL
void fd2_sdl_set_palette_from_6bit(int start, int end, const byte* palette_6bit) {
    byte palette8[768];
    for (int i = start; i <= end && i < 256; i++) {
        byte r6 = palette_6bit[i * 3] & 0x3F;
        byte g6 = palette_6bit[i * 3 + 1] & 0x3F;
        byte b6 = palette_6bit[i * 3 + 2] & 0x3F;
        palette8[i * 3] = (r6 << 2) | (r6 >> 4);
        palette8[i * 3 + 1] = (g6 << 2) | (g6 >> 4);
        palette8[i * 3 + 2] = (b6 << 2) | (b6 >> 4);
    }
    fd2_sdl_set_palette_6bit(start, end, palette8);
}
#endif
