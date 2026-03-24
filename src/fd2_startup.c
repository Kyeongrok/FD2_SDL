#include "fd2_types.h"
#include "fd2_dat.h"
#include "fd2_rle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

typedef struct {
    const char* filename;
    int cached_handle;
    byte* cached_data;
} DATCache;

static DATCache g_dat_cache[16];
static int g_dat_cache_count = 0;

static DATCache* get_cache(const char* filename) {
    for (int i = 0; i < g_dat_cache_count; i++) {
        if (strcmp(g_dat_cache[i].filename, filename) == 0) {
            return &g_dat_cache[i];
        }
    }
    if (g_dat_cache_count < 16) {
        g_dat_cache[g_dat_cache_count].filename = filename;
        g_dat_cache[g_dat_cache_count].cached_handle = 0;
        g_dat_cache[g_dat_cache_count].cached_data = NULL;
        return &g_dat_cache[g_dat_cache_count++];
    }
    return NULL;
}

int fd2_load_dat_resource(const char* filename, int prev_buf, int resource_idx, byte** out_data, dword* out_size) {
    if (prev_buf) {
        free((void*)prev_buf);
    }
    
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("\n\n File not found %s!!! \n\n", filename);
        return 0;
    }
    
    byte header[8];
    fseek(fp, 4 * resource_idx + 6, SEEK_SET);
    if (fread(header, 1, 8, fp) != 8) {
        fclose(fp);
        return 0;
    }
    
    dword offset = *(dword*)header;
    dword end_offset = *(dword*)(header + 4);
    dword data_size = end_offset - offset;
    
    byte* data = (byte*)malloc(data_size);
    if (!data) {
        printf("Out of Memory at Load %s Number:%d!!\n", filename, resource_idx);
        fclose(fp);
        return 0;
    }
    
    fseek(fp, offset, SEEK_SET);
    fread(data, 1, data_size, fp);
    fclose(fp);
    
    if (out_data) *out_data = data;
    if (out_size) *out_size = data_size;
    
    return (int)data;
}

int fd2_load_resource_cached(int* cache_ptr, const char* filename, int resource_idx, byte** out_data, dword* out_size) {
    byte* data = NULL;
    dword size = 0;
    int result = fd2_load_dat_resource(filename, *cache_ptr, resource_idx, &data, &size);
    *cache_ptr = result;
    if (out_data) *out_data = data;
    if (out_size) *out_size = size;
    return result;
}

typedef void (*PresentFunc)(byte* screen);
typedef void (*SetPaletteFunc)(byte* palette, int start, int end, int brightness);
typedef void (*DelayFunc)(int ms);

typedef struct {
    PresentFunc present;
    SetPaletteFunc set_palette;
    DelayFunc delay;
} StartupCallbacks;

void fd2_startup_phase0(int* palette_handle, byte* screen, byte* palette, StartupCallbacks* cb) {
    printf("[启动] Phase 0: 加载初始调色板\n");
    
    *palette_handle = fd2_load_dat_resource("FDOTHER.DAT", *palette_handle, 76, NULL, NULL);
    
    if (palette && cb && cb->set_palette) {
        cb->set_palette(palette, 0, 255, 64);
    }
}

void fd2_startup_phase1(int* palette_handle, byte* screen, byte* palette, StartupCallbacks* cb) {
    printf("[启动] Phase 1: 绘制欢迎画面\n");
    
    dword size;
    byte* data = NULL;
    int res74 = fd2_load_dat_resource("FDOTHER.DAT", 0, 74, &data, &size);
    
    if (data && size > 0) {
        fd2_decompress_rle(data, size, screen, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, -1);
        free(data);
    }
    free((void*)res74);
    
    if (palette && cb && cb->set_palette) {
        cb->set_palette(palette, 0, 255, 64);
    }
}

void fd2_startup_phase2(int* palette_handle, byte* screen, byte* palette, StartupCallbacks* cb) {
    printf("[启动] Phase 2: 加载调色板资源99\n");
    
    *palette_handle = fd2_load_dat_resource("FDOTHER.DAT", *palette_handle, 99, NULL, NULL);
    
    memset(screen, 0, SCREEN_SIZE);
    
    if (palette && cb && cb->set_palette) {
        cb->set_palette(palette, 0, 255, 0);
    }
}

void fd2_startup_phase3(int* palette_handle, byte* screen, byte* palette, StartupCallbacks* cb) {
    printf("[启动] Phase 3: 加载调色板资源101\n");
    
    *palette_handle = fd2_load_dat_resource("FDOTHER.DAT", *palette_handle, 101, NULL, NULL);
    
    if (palette && cb && cb->set_palette) {
        cb->set_palette(palette, 0, 255, 64);
    }
}

int fd2_startup_load_bar_frames(byte* bar_buf) {
    printf("[启动] 加载条形动画帧 (资源69-73)\n");
    
    if (!bar_buf) {
        bar_buf = (byte*)malloc(5 * 147 * SCREEN_WIDTH);
        if (!bar_buf) return 0;
    }
    
    memset(bar_buf, 0, 5 * 147 * SCREEN_WIDTH);
    
    for (int i = 0; i < 5; i++) {
        dword size;
        byte* data = NULL;
        int handle = fd2_load_dat_resource("FDOTHER.DAT", 0, 69 + i, &data, &size);
        
        if (data && size > 0) {
            byte* decoded = (byte*)malloc(SCREEN_WIDTH * 147);
            if (decoded) {
                memset(decoded, 0, SCREEN_WIDTH * 147);
                fd2_decompress_rle(data, size, decoded, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, -1);
                memcpy(bar_buf + i * 147 * SCREEN_WIDTH, decoded, 147 * SCREEN_WIDTH);
                free(decoded);
            }
        }
        
        if (data) free(data);
        free((void*)handle);
    }
    
    return 1;
}

void fd2_startup_bar_animation(byte* screen, byte* bar_buf, int frames, DelayFunc delay, PresentFunc present) {
    printf("[启动] 条形动画 (%d帧)\n", frames);
    
    for (int f = 0; f < frames; f++) {
        int bar_idx = (f / 4) % 5;
        int src_y = (f % 4) * 147;
        int dst_y = 0;
        
        while (src_y < (bar_idx + 1) * 147 && dst_y < SCREEN_HEIGHT) {
            byte* src_row = bar_buf + src_y * SCREEN_WIDTH;
            byte* dst_row = screen + dst_y * SCREEN_WIDTH;
            memcpy(dst_row, src_row, SCREEN_WIDTH);
            src_y++;
            dst_y++;
        }
        
        if (present) present(screen);
        if (delay) delay(16);
    }
}

void fd2_startup_fade_out(byte* palette, int brightness, SetPaletteFunc set_palette, DelayFunc delay) {
    printf("[启动] 淡出效果\n");
    
    for (int b = brightness; b >= 0; b--) {
        if (set_palette) set_palette(palette, 0, 255, b);
        if (delay) delay(8);
    }
    
    if (delay) delay(100);
}

int fd2_startup_load_menu_resources(int* res7_handle, int* res8_handle) {
    printf("[启动] 加载菜单资源\n");
    
    *res7_handle = fd2_load_dat_resource("FDOTHER.DAT", 0, 7, NULL, NULL);
    *res8_handle = fd2_load_dat_resource("FDOTHER.DAT", 0, 8, NULL, NULL);
    
    return (*res7_handle && *res8_handle) ? 1 : 0;
}

int fd2_run_startup_sequence(byte* screen, byte* palette, StartupCallbacks* cb) {
    printf("\n============================================\n");
    printf("  FD2.exe 启动序列\n");
    printf("  基于IDA Pro逆向工程分析\n");
    printf("============================================\n\n");
    
    int palette_handle = 0;
    int res7_handle = 0;
    int res8_handle = 0;
    byte* bar_buf = NULL;
    int result = 0;
    
    fd2_startup_phase0(&palette_handle, screen, palette, cb);
    
    fd2_startup_phase1(&palette_handle, screen, palette, cb);
    
    fd2_startup_phase2(&palette_handle, screen, palette, cb);
    
    fd2_startup_phase3(&palette_handle, screen, palette, cb);
    
    if (!fd2_startup_load_bar_frames(bar_buf)) {
        printf("[错误] 条形动画加载失败\n");
        goto cleanup;
    }
    
    fd2_startup_bar_animation(screen, bar_buf, 535, cb->delay, cb->present);
    
    fd2_startup_fade_out(palette, 63, cb->set_palette, cb->delay);
    
    if (!fd2_startup_load_menu_resources(&res7_handle, &res8_handle)) {
        printf("[错误] 菜单资源加载失败\n");
        goto cleanup;
    }
    
    result = 1;
    
cleanup:
    if (bar_buf) free(bar_buf);
    free((void*)palette_handle);
    free((void*)res7_handle);
    free((void*)res8_handle);
    
    printf("[启动] 启动序列完成\n\n");
    return result;
}
