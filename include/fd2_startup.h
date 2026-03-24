#ifndef FD2_STARTUP_H
#define FD2_STARTUP_H

#include "fd2_types.h"

typedef struct {
    void (*present)(byte* screen);
    void (*set_palette)(byte* palette, int start, int end, int brightness);
    void (*delay)(int ms);
} StartupCallbacks;

int fd2_load_dat_resource(const char* filename, int prev_buf, int resource_idx, byte** out_data, dword* out_size);
int fd2_load_resource_cached(int* cache_ptr, const char* filename, int resource_idx, byte** out_data, dword* out_size);

int fd2_decompress_rle(byte* src, int src_size, byte* dst, int dst_stride, int dst_x, int dst_y, int dst_width, int value_fill);

void fd2_startup_phase0(int* palette_handle, byte* screen, byte* palette, StartupCallbacks* cb);
void fd2_startup_phase1(int* palette_handle, byte* screen, byte* palette, StartupCallbacks* cb);
void fd2_startup_phase2(int* palette_handle, byte* screen, byte* palette, StartupCallbacks* cb);
void fd2_startup_phase3(int* palette_handle, byte* screen, byte* palette, StartupCallbacks* cb);

int fd2_startup_load_bar_frames(byte* bar_buf);
void fd2_startup_bar_animation(byte* screen, byte* bar_buf, int frames, void (*delay)(int), void (*present)(byte*));
void fd2_startup_fade_out(byte* palette, int brightness, void (*set_palette)(byte*, int, int, int), void (*delay)(int));

int fd2_startup_load_menu_resources(int* res7_handle, int* res8_handle);

int fd2_run_startup_sequence(byte* screen, byte* palette, StartupCallbacks* cb);

#endif
