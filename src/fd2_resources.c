#include "fd2_resources.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <dirent.h>

/* BMP文件头结构 (简化版) */
#pragma pack(push, 1)
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPFileHeader;

typedef struct {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;
#pragma pack(pop)

/* ============================================================
 * 图块集加载 (从BMP文件)
 * ============================================================ */
int tileset_load(TileSet *ts, const char *bmp_dir) {
    if (!ts || !bmp_dir) return -1;
    
    printf("[tileset] 从BMP加载: %s\n", bmp_dir);
    fflush(stdout);
    
    ts->tile_width = 24;
    ts->tile_height = 24;
    ts->stride = 72;  /* BMP每行24像素*3字节=72字节，需要>=72 */
    
    int max_tiles = 16704;
    
    ts->pixel_data = (byte*)malloc((size_t)max_tiles * ts->stride * ts->tile_height);
    if (!ts->pixel_data) {
        fprintf(stderr, "图块内存分配失败\n");
        return -1;
    }
    memset(ts->pixel_data, 0, (size_t)max_tiles * ts->stride * ts->tile_height);
    
    ts->count = 0;
    
    char fdshap_path[512];
    snprintf(fdshap_path, sizeof(fdshap_path), "%s/FDSHAP_BMP", bmp_dir);
    
    DIR *d = opendir(fdshap_path);
    if (!d) {
        fprintf(stderr, "无法打开FDSHAP目录: %s\n", fdshap_path);
        free(ts->pixel_data);
        ts->pixel_data = NULL;
        return -1;
    }
    
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        /* 检查是否是目录 - 通过尝试打开子目录 */
        char subdir_path[512];
        snprintf(subdir_path, sizeof(subdir_path), "%s/%s", fdshap_path, entry->d_name);
        
        /* 检查目录名是否是 fdshap_XXX 格式 */
        if (strncmp(entry->d_name, "fdshap_", 7) != 0) continue;
        
        /* 尝试打开子目录验证是否是目录 */
        DIR *subd = opendir(subdir_path);
        if (!subd) continue;
        closedir(subd);
        
        int folder_idx = atoi(entry->d_name + 7);
        
        DIR *tile_d = opendir(subdir_path);
        if (!tile_d) continue;
        
        struct dirent *tile_entry;
        while ((tile_entry = readdir(tile_d)) != NULL) {
            /* 只处理 .bmp 文件 */
            if (strstr(tile_entry->d_name, ".bmp") == NULL) continue;
            if (tile_entry->d_name[0] == '.') continue;
            
            char *underscore = strrchr(tile_entry->d_name, '_');
            if (!underscore) continue;
            int tile_idx = atoi(underscore + 1);
            
            int global_tile_id = folder_idx * 288 + tile_idx;
            if (global_tile_id >= max_tiles) continue;
            
            char bmp_path[512];
            snprintf(bmp_path, sizeof(bmp_path), "%s/%s", subdir_path, tile_entry->d_name);
            
            FILE *bf = fopen(bmp_path, "rb");
            if (!bf) continue;
            
            BMPFileHeader fh;
            BMPInfoHeader ih;
            if (fread(&fh, sizeof(fh), 1, bf) == 1 &&
                fread(&ih, sizeof(ih), 1, bf) == 1) {
                
                if (ih.biWidth == 24 && ih.biHeight == 24 && ih.biBitCount == 24) {
                    byte *tile_data = ts->pixel_data + (size_t)global_tile_id * ts->stride * ts->tile_height;
                    
                    /* BMP是BGR格式且bottom-up，需要倒序读取并交换R/B */
                    for (int y = 23; y >= 0; y--) {
                        byte *row = tile_data + y * ts->stride;
                        byte bgr[24 * 3];
                        fread(bgr, 1, 24 * 3, bf);
                        
                        /* BGR → RGB 转换 */
                        for (int x = 0; x < 24; x++) {
                            row[x * 3 + 0] = bgr[x * 3 + 2];  /* R */
                            row[x * 3 + 1] = bgr[x * 3 + 1];  /* G */
                            row[x * 3 + 2] = bgr[x * 3 + 0];  /* B */
                        }
                    }
                    
                    ts->count = (global_tile_id + 1 > ts->count) ? global_tile_id + 1 : ts->count;
                }
            }
            fclose(bf);
        }
        closedir(tile_d);
    }
    closedir(d);
    
    printf("加载图块集: %d 个图块, %dx%d\n", ts->count, ts->tile_width, ts->tile_height);
    return 0;
}

void tileset_free(TileSet *ts) {
    free(ts->pixel_data);
    ts->pixel_data = NULL;
    ts->count = 0;
}

/* ============================================================
 * 图标集加载
 * ============================================================ */
int iconset_load(IconSet *is, const char *bin_path) {
    FILE *f = fopen(bin_path, "rb");
    if (!f) {
        fprintf(stderr, "无法打开图标文件: %s\n", bin_path);
        return -1;
    }

    /* 读取头部: [count(4)][width(2)][height(2)] */
    uint32_t count;
    int16_t w, h;
    if (fread(&count, 4, 1, f) != 1) { fclose(f); return -1; }
    if (fread(&w, 2, 1, f) != 1) { fclose(f); return -1; }
    if (fread(&h, 2, 1, f) != 1) { fclose(f); return -1; }

    is->count = (int)count;
    is->width = w;
    is->height = h;

    size_t pixel_size = (size_t)count * w * h * 3;
    is->pixel_data = (byte*)malloc(pixel_size);
    if (!is->pixel_data) {
        fclose(f);
        return -1;
    }

    if (fread(is->pixel_data, 1, pixel_size, f) != pixel_size) {
        fprintf(stderr, "图标数据读取不完整\n");
        free(is->pixel_data);
        is->pixel_data = NULL;
        fclose(f);
        return -1;
    }

    fclose(f);
    printf("加载图标集: %d 个图标, %dx%d\n", is->count, is->width, is->height);
    return 0;
}

void iconset_free(IconSet *is) {
    free(is->pixel_data);
    is->pixel_data = NULL;
    is->count = 0;
}

/* ============================================================
 * 地图数据加载
 * ============================================================ */
int mapdata_load(MapData *md, int map_index, const char *base_dir) {
    char path[256];
    snprintf(path, sizeof(path), "%s/map_%03d.bin", base_dir, map_index);

    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "无法打开地图文件: %s\n", path);
        return -1;
    }

    int16_t w, h;
    if (fread(&w, 2, 1, f) != 1) { fclose(f); return -1; }
    if (fread(&h, 2, 1, f) != 1) { fclose(f); return -1; }

    md->width = (byte)w;
    md->height = (byte)h;

    int tile_count = w * h;
    md->tile_ids = (short*)malloc(tile_count * sizeof(short));
    if (!md->tile_ids) {
        fclose(f);
        return -1;
    }

    for (int i = 0; i < tile_count; i++) {
        if (fread(&md->tile_ids[i], 2, 1, f) != 1) {
            md->tile_ids[i] = 0;
        }
    }

    fclose(f);
    printf("加载地图 %d: %dx%d = %d 图块\n", map_index, md->width, md->height, tile_count);
    return 0;
}

void mapdata_free(MapData *md) {
    free(md->tile_ids);
    md->tile_ids = NULL;
    md->width = 0;
    md->height = 0;
}

void mapdata_render(const MapData *map, const TileSet *tiles,
                    byte *screen, int screen_w, int screen_h,
                    const Palette *pal) {
    if (!map || !tiles || !screen) return;

    printf("[mapdata_render] map=%dx%d, tiles=%d, screen=%dx%d\n", 
           map->width, map->height, tiles->count, screen_w, screen_h);
    fflush(stdout);

    for (int ty = 0; ty < map->height; ty++) {
        for (int tx = 0; tx < map->width; tx++) {
            int tile_id = map->tile_ids[ty * map->width + tx];
            if (tile_id < 0 || tile_id >= tiles->count) continue;

            const byte *tile_rgb = tileset_get(tiles, tile_id);
            if (!tile_rgb) continue;

            int sx = tx * tiles->tile_width;
            int sy = ty * tiles->tile_height;

            for (int dy = 0; dy < tiles->tile_height; dy++) {
                for (int dx = 0; dx < tiles->tile_width; dx++) {
                    int px = sx + dx;
                    int py = sy + dy;
                    if (px >= screen_w || py >= screen_h) continue;

                    int ri = dy * tiles->stride + dx * 3;
                    byte r = tile_rgb[ri];
                    byte g = tile_rgb[ri + 1];
                    byte b = tile_rgb[ri + 2];

                    int best_idx = 0;
                    int best_dist = 0x7FFFFFFF;
                    for (int ci = 0; ci < PALETTE_SIZE; ci++) {
                        int dr = r - pal->colors[ci][0];
                        int dg = g - pal->colors[ci][1];
                        int db = b - pal->colors[ci][2];
                        int dist = dr*dr + dg*dg + db*db;
                        if (dist < best_dist) {
                            best_dist = dist;
                            best_idx = ci;
                            if (dist == 0) break;
                        }
                    }
                    screen[py * screen_w + px] = (byte)best_idx;
                }
            }
        }
    }
}

/* ============================================================
 * 全局资源管理器
 * ============================================================ */
int fd2_resources_init(FD2Resources *res, const char *base_dir) {
    if (!res || !base_dir) return -1;

    memset(res, 0, sizeof(*res));

    /* 默认调色板 (6位) */
    byte default_pal_6bit[768];
    for (int i = 0; i < 768; i++) {
        default_pal_6bit[i] = (byte)(i / 3);
    }

    /* 加载FDOTHER.DAT获取调色板 */
    char fdother_path[256];
    snprintf(fdother_path, sizeof(fdother_path), "%s/FDOTHER.DAT", base_dir);
    FILE* fp = fopen(fdother_path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long fdsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        byte* fddata = (byte*)malloc(fdsize);
        if (fddata && fread(fddata, 1, fdsize, fp) == (size_t)fdsize) {
            /* FDOTHER.DAT 格式: "LLLLLL" + count(4) + offsets[] (104 entries like BG.DAT) */
            uint32_t offsets[104];
            for (int i = 0; i < 104; i++) {
                offsets[i] = *(uint32_t*)(fddata + 6 + i * 4);
            }
            uint32_t pal_start = offsets[0];
            uint32_t pal_end = offsets[1];
            printf("[资源] FDOTHER.DAT: 调色板 offset=%u (%u bytes)\n", pal_start, pal_end - pal_start);
            if (pal_start < (uint32_t)fdsize && pal_end <= (uint32_t)fdsize && pal_end > pal_start && pal_start > 0) {
                palette_init_6bit(&res->palette, fddata + pal_start);
            } else {
                printf("[资源] FDOTHER.DAT: 调色板偏移无效，使用默认\n");
                palette_init_6bit(&res->palette, default_pal_6bit);
            }
        }
        if (fddata) free(fddata);
        fclose(fp);
    } else {
        printf("[资源] 无法打开FDOTHER.DAT，使用默认调色板\n");
        palette_init_6bit(&res->palette, default_pal_6bit);
    }

    /* 加载图块集 - 从BMP目录 */
    if (tileset_load(&res->tiles, base_dir) != 0) {
        fprintf(stderr, "警告: 图块集加载失败\n");
    }

    /* 加载图标集 */
    char icon_path[256];
    snprintf(icon_path, sizeof(icon_path), "%s/fd2_icons.bin", base_dir);
    if (iconset_load(&res->icons, icon_path) != 0) {
        fprintf(stderr, "警告: 图标集加载失败\n");
    }

    res->loaded = 1;
    printf("FD2资源管理器初始化完成\n");
    return 0;
}

void fd2_resources_free(FD2Resources *res) {
    if (!res) return;
    tileset_free(&res->tiles);
    iconset_free(&res->icons);
    res->loaded = 0;
}
