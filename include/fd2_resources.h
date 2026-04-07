#ifndef FD2_RESOURCES_H
#define FD2_RESOURCES_H

#include "fd2_types.h"
#include "fd2_image.h"

#define FD2_ICON_COUNT 1680
#define FD2_TILE_COUNT 16704
#define FD2_TILE_WIDTH 24
#define FD2_TILE_HEIGHT 24
#define FD2_MAP_COUNT 100
#define FD2_MAX_MAP_TILES 64

/* ============================================================
 * 资源文件路径 (相对于游戏目录)
 * ============================================================ */
#define FD2_RESOURCES_DIR "data/exported/"
#define FD2_ICONS_BIN     FD2_RESOURCES_DIR "fd2_icons.bin"
#define FD2_TILES_BIN     FD2_RESOURCES_DIR "fd2_tiles.bin"
#define FD2_TEXTS_TXT     FD2_RESOURCES_DIR "fd2_texts.txt"

/* ============================================================
 * 图块集 (FDSHAP)
 * ============================================================ */
typedef struct {
    int count;
    int tile_width;
    int tile_height;
    int stride;  /* 每行字节数 (32字节行距) */
    byte *pixel_data;  /* 展平的RGB数据: count * stride * h */
} TileSet;

int tileset_load(TileSet *ts, const char *bin_path);
void tileset_free(TileSet *ts);

/* 获取图块RGB数据指针 (使用stride) */
static inline const byte *tileset_get(const TileSet *ts, int index) {
    if (index < 0 || index >= ts->count) return NULL;
    return ts->pixel_data + (size_t)index * ts->stride * ts->tile_height;
}

/* 将图块写入屏幕缓冲区 (RGB -> 调色板索引匹配) */
void tileset_blit_to_screen(const TileSet *ts, int tile_index,
                            byte *screen, int screen_w, int screen_h,
                            int sx, int sy, const Palette *pal);

/* ============================================================
 * 图标 (FDICON)
 * ============================================================ */
typedef struct {
    int count;
    int width;
    int height;
    byte *pixel_data;  /* 展平的RGB数据 */
} IconSet;

int iconset_load(IconSet *is, const char *bin_path);
void iconset_free(IconSet *is);

static inline const byte *iconset_get(const IconSet *is, int index) {
    if (index < 0 || index >= is->count) return NULL;
    return is->pixel_data + (size_t)index * is->width * is->height * 3;
}

/* ============================================================
 * 地图 (FDFIELD)
 * ============================================================ */
typedef struct {
    byte width;
    byte height;
    short *tile_ids;  /* width * height 个图块ID */
} MapData;

int mapdata_load(MapData *md, int map_index, const char *base_dir);
void mapdata_free(MapData *md);

/* 将地图图块ID矩阵渲染到屏幕 */
void mapdata_render(const MapData *map, const TileSet *tiles,
                    byte *screen, int screen_w, int screen_h,
                    const Palette *pal);

/* ============================================================
 * 全局资源管理器 (单例)
 * ============================================================ */
typedef struct {
    TileSet tiles;
    IconSet icons;
    Palette palette;
    int loaded;
} FD2Resources;

int fd2_resources_init(FD2Resources *res, const char *base_dir);
void fd2_resources_free(FD2Resources *res);

/* 全局实例访问 */
FD2Resources *fd2_resources_get(void);

#endif /* FD2_RESOURCES_H */
