/**
 * FD2 地图系统实现
 * 基于IDA Pro逆向工程分析
 * 
 * FDFIELD.DAT结构:
 * - 100个关卡，每关3个资源
 * - 资源0: RLE压缩的地图数据
 * - 资源1: 关卡头信息
 * - 资源2: 形状索引
 */

#include "../include/fd2_map.h"
#include "../include/fd2_dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

const byte g_terrain_move_cost[TERRAIN_MAX] = {
    1,  // TERRAIN_PLAINS
    2,  // TERRAIN_FOREST
    3,  // TERRAIN_MOUNTAIN
    99, // TERRAIN_WATER (不可行走)
    99, // TERRAIN_WALL
    1,  // TERRAIN_ROAD
    1,  // TERRAIN_BRIDGE
    1,  // TERRAIN_CASTLE
    1   // TERRAIN_VILLAGE
};

const byte g_terrain_defense_bonus[TERRAIN_MAX] = {
    0,   // TERRAIN_PLAINS
    2,   // TERRAIN_FOREST
    4,   // TERRAIN_MOUNTAIN
    0,   // TERRAIN_WATER
    0,   // TERRAIN_WALL
    0,   // TERRAIN_ROAD
    0,   // TERRAIN_BRIDGE
    3,   // TERRAIN_CASTLE
    1    // TERRAIN_VILLAGE
};

static int map_decompress_rle(const byte* src, int src_size, byte* dst, int dst_width, int dst_height) {
    if (!src || src_size < 4 || !dst) return -1;
    
    unsigned short row_bytes = *(unsigned short*)src;
    unsigned short num_rows = *(unsigned short*)(src + 2);
    
    if (row_bytes > dst_width || num_rows > dst_height) return -1;
    
    src += 4;
    int src_left = src_size - 4;
    int row_gap = dst_width - row_bytes;
    
    for (int row_iter = num_rows; row_iter > 0; row_iter--) {
        unsigned short remaining = row_bytes;
        
        while (remaining > 0 && src_left > 0) {
            unsigned char val = *src++;
            src_left--;
            
            unsigned int num = (val & 0x3F) + 1;
            if (num > remaining) num = remaining;
            
            if ((val & 0x80) == 0) {
                if (src_left >= (int)num) {
                    memcpy(dst, src, num);
                    src += num;
                    src_left -= num;
                    dst += num;
                }
            } else if ((val & 0x40) == 0) {
                memset(dst, 0, num);
                dst += num;
            } else if ((val & 0x20) == 0) {
                for (unsigned int i = 0; i < num; i++) {
                    *dst++ = *src++;
                    src_left--;
                }
            } else {
                byte fill = *src++;
                src_left--;
                remaining -= num * 2;
                for (unsigned int i = 0; i < num; i++) {
                    *dst++ = fill;
                }
                continue;
            }
            remaining -= num;
        }
        
        dst += row_gap;
    }
    
    return 0;
}

int map_load_level(BattleMap* map, const byte* field_data, dword field_size, const LevelHeader* header) {
    if (!map || !field_data || field_size < 4) return -1;
    
    memset(map, 0, sizeof(BattleMap));
    
    map->width = *(word*)field_data;
    map->height = *(word*)(field_data + 2);
    
    if (map->width > MAX_MAP_WIDTH || map->height > MAX_MAP_HEIGHT) {
        printf("[地图] 尺寸过大: %dx%d\n", map->width, map->height);
        return -1;
    }
    
    map->tiles = (MapTile*)malloc(map->width * map->height * sizeof(MapTile));
    if (!map->tiles) {
        printf("[地图] 内存分配失败\n");
        return -1;
    }
    
    byte* decompressed = (byte*)malloc(map->width * map->height);
    if (!decompressed) {
        free(map->tiles);
        map->tiles = NULL;
        return -1;
    }
    
    memset(decompressed, 0, map->width * map->height);
    
    fd_decompress_rle(field_data + 4, field_size - 4, decompressed, map->width, map->height, -1);
    
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            MapTile* tile = &map->tiles[y * map->width + x];
            byte tile_id = decompressed[y * map->width + x];
            
            tile->tile_id = tile_id;
            tile->terrain = TERRAIN_PLAINS;
            tile->height = 0;
            tile->flags = TILE_WALKABLE;
            tile->move_cost = 1;
            tile->defense_bonus = 0;
            tile->attack_bonus = 0;
            
            if (tile_id == 0) {
                tile->terrain = TERRAIN_PLAINS;
                tile->flags = TILE_WALKABLE;
                tile->move_cost = 1;
            } else if (tile_id >= 1 && tile_id <= 3) {
                tile->terrain = TERRAIN_FOREST;
                tile->flags = TILE_WALKABLE;
                tile->move_cost = 2;
                tile->defense_bonus = 2;
            } else if (tile_id >= 4 && tile_id <= 6) {
                tile->terrain = TERRAIN_MOUNTAIN;
                tile->flags = TILE_FLYABLE;
                tile->move_cost = 3;
                tile->defense_bonus = 4;
            } else if (tile_id >= 7 && tile_id <= 9) {
                tile->terrain = TERRAIN_WATER;
                tile->flags = TILE_EMPTY;
                tile->move_cost = 99;
            } else if (tile_id >= 10) {
                tile->terrain = TERRAIN_ROAD;
                tile->flags = TILE_WALKABLE;
                tile->move_cost = 1;
            }
        }
    }
    
    free(decompressed);
    
    if (header) {
        map->header = (LevelHeader*)malloc(sizeof(LevelHeader));
        if (map->header) {
            memcpy(map->header, header, sizeof(LevelHeader));
        }
    }
    
    map->raw_data = (byte*)malloc(field_size);
    if (map->raw_data) {
        memcpy(map->raw_data, field_data, field_size);
        map->raw_size = field_size;
    }
    
    printf("[地图] 加载成功: %dx%d\n", map->width, map->height);
    
    return 0;
}

void map_free(BattleMap* map) {
    if (!map) return;
    
    if (map->tiles) {
        free(map->tiles);
        map->tiles = NULL;
    }
    
    if (map->header) {
        free(map->header);
        map->header = NULL;
    }
    
    if (map->raw_data) {
        free(map->raw_data);
        map->raw_data = NULL;
    }
    
    memset(map, 0, sizeof(BattleMap));
}

MapTile* map_get_tile(BattleMap* map, byte x, byte y) {
    if (!map || !map_is_in_bounds(map, x, y)) return NULL;
    return &map->tiles[y * map->width + x];
}

bool map_is_walkable(BattleMap* map, byte x, byte y) {
    MapTile* tile = map_get_tile(map, x, y);
    if (!tile) return false;
    return tile->move_cost < 99;
}

bool map_is_in_bounds(BattleMap* map, byte x, byte y) {
    if (!map || !map->tiles) return false;
    return x < map->width && y < map->height;
}

byte map_get_move_cost(BattleMap* map, byte x, byte y) {
    MapTile* tile = map_get_tile(map, x, y);
    if (!tile) return 99;
    return tile->move_cost;
}

byte map_get_terrain_defense(BattleMap* map, byte x, byte y) {
    MapTile* tile = map_get_tile(map, x, y);
    if (!tile) return 0;
    return tile->defense_bonus;
}

static int map_heuristic(byte x1, byte y1, byte x2, byte y2) {
    int dx = abs((int)x1 - (int)x2);
    int dy = abs((int)y1 - (int)y2);
    return dx + dy;
}

int map_find_path(BattleMap* map, byte start_x, byte start_y, byte end_x, byte end_y, Path* path, byte move_range) {
    if (!map || !path || !map_is_in_bounds(map, start_x, start_y) || !map_is_in_bounds(map, end_x, end_y)) {
        return -1;
    }
    
    memset(path, 0, sizeof(Path));
    path->max_nodes = 256;
    path->nodes = (PathNode*)malloc(path->max_nodes * sizeof(PathNode));
    if (!path->nodes) return -1;
    
    typedef struct {
        byte x, y;
        int g_cost;
        int h_cost;
        int f_cost;
        byte parent_idx;
    } Node;
    
    Node* open_list = (Node*)malloc(MAX_MAP_WIDTH * MAX_MAP_HEIGHT * sizeof(Node));
    Node* closed_list = (Node*)malloc(MAX_MAP_WIDTH * MAX_MAP_HEIGHT * sizeof(Node));
    
    if (!open_list || !closed_list) {
        free(open_list);
        free(closed_list);
        return -1;
    }
    
    int open_count = 0;
    int closed_count = 0;
    
    open_list[open_count++] = (Node){start_x, start_y, 0, map_heuristic(start_x, start_y, end_x, end_y), 
                                        map_heuristic(start_x, start_y, end_x, end_y), 0};
    
    const byte dirs[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    
    while (open_count > 0) {
        int lowest_idx = 0;
        for (int i = 1; i < open_count; i++) {
            if (open_list[i].f_cost < open_list[lowest_idx].f_cost) {
                lowest_idx = i;
            }
        }
        
        Node current = open_list[lowest_idx];
        
        for (int i = lowest_idx; i < open_count - 1; i++) {
            open_list[i] = open_list[i + 1];
        }
        open_count--;
        
        if (current.x == end_x && current.y == end_y) {
            path->nodes[path->node_count++] = (PathNode){end_x, end_y, end_x, end_y, 0};
            
            byte px = end_x, py = end_y;
            
            while (!(px == start_x && py == start_y)) {
                bool found_parent = false;
                for (int i = 0; i < closed_count; i++) {
                    if (closed_list[i].x == px && closed_list[i].y == py) {
                        byte parent_idx = closed_list[i].parent_idx;
                        if (parent_idx < closed_count) {
                            byte nx = closed_list[parent_idx].x;
                            byte ny = closed_list[parent_idx].y;
                            
                            MapTile* tile = map_get_tile(map, nx, ny);
                            byte mc = tile ? tile->move_cost : 1;
                            
                            path->nodes[path->node_count++] = (PathNode){nx, ny, px, py, mc};
                            px = nx;
                            py = ny;
                            found_parent = true;
                        }
                        break;
                    }
                }
                if (!found_parent) break;
            }
            
            free(open_list);
            free(closed_list);
            return path->node_count;
        }
        
        closed_list[closed_count++] = current;
        
        if (current.g_cost >= move_range) continue;
        
        for (int i = 0; i < 4; i++) {
            byte nx = current.x + dirs[i][0];
            byte ny = current.y + dirs[i][1];
            
            if (!map_is_in_bounds(map, nx, ny)) continue;
            if (!map_is_walkable(map, nx, ny)) continue;
            
            MapTile* tile = map_get_tile(map, nx, ny);
            byte move_cost = tile ? tile->move_cost : 1;
            
            int new_g = current.g_cost + move_cost;
            
            bool in_closed = false;
            for (int j = 0; j < closed_count; j++) {
                if (closed_list[j].x == nx && closed_list[j].y == ny) {
                    in_closed = true;
                    break;
                }
            }
            if (in_closed) continue;
            
            bool in_open = false;
            for (int j = 0; j < open_count; j++) {
                if (open_list[j].x == nx && open_list[j].y == ny) {
                    in_open = true;
                    if (new_g < open_list[j].g_cost) {
                        open_list[j].g_cost = new_g;
                        open_list[j].f_cost = new_g + open_list[j].h_cost;
                        open_list[j].parent_idx = closed_count - 1;
                    }
                    break;
                }
            }
            
            if (!in_open) {
                open_list[open_count++] = (Node){
                    nx, ny, new_g, 
                    map_heuristic(nx, ny, end_x, end_y),
                    new_g + map_heuristic(nx, ny, end_x, end_y),
                    (byte)(closed_count - 1)
                };
            }
        }
    }
    
    free(open_list);
    free(closed_list);
    
    return -1;
}

void map_get_reachable_tiles(BattleMap* map, byte x, byte y, byte move_range, bool reachable[MAX_MAP_WIDTH][MAX_MAP_HEIGHT]) {
    if (!map) return;
    
    memset(reachable, 0, MAX_MAP_WIDTH * MAX_MAP_HEIGHT * sizeof(bool));
    
    if (!map_is_in_bounds(map, x, y)) return;
    
    typedef struct {
        byte x, y;
        byte cost;
    } QueueNode;
    
    QueueNode queue[MAX_MAP_WIDTH * MAX_MAP_HEIGHT];
    int queue_head = 0, queue_tail = 0;
    
    queue[queue_tail++] = (QueueNode){x, y, 0};
    reachable[y][x] = true;
    
    while (queue_head < queue_tail) {
        QueueNode current = queue[queue_head++];
        
        const byte dirs[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
        
        for (int i = 0; i < 4; i++) {
            byte nx = current.x + dirs[i][0];
            byte ny = current.y + dirs[i][1];
            
            if (!map_is_in_bounds(map, nx, ny)) continue;
            if (!map_is_walkable(map, nx, ny)) continue;
            if (reachable[ny][nx]) continue;
            
            MapTile* tile = map_get_tile(map, nx, ny);
            byte move_cost = tile ? tile->move_cost : 1;
            
            if (current.cost + move_cost <= move_range) {
                reachable[ny][nx] = true;
                queue[queue_tail++] = (QueueNode){nx, ny, (byte)(current.cost + move_cost)};
            }
        }
    }
}

void map_get_attackable_tiles(BattleMap* map, byte x, byte y, byte attack_range, bool attackable[MAX_MAP_WIDTH][MAX_MAP_HEIGHT]) {
    if (!map) return;
    
    memset(attackable, 0, MAX_MAP_WIDTH * MAX_MAP_HEIGHT * sizeof(bool));
    
    if (!map_is_in_bounds(map, x, y)) return;
    
    for (int dy = -attack_range; dy <= attack_range; dy++) {
        for (int dx = -attack_range; dx <= attack_range; dx++) {
            if (dx == 0 && dy == 0) continue;
            
            byte nx = x + dx;
            byte ny = y + dy;
            
            if (!map_is_in_bounds(map, nx, ny)) continue;
            
            int dist = abs(dx) + abs(dy);
            if (dist <= attack_range) {
                attackable[ny][nx] = true;
            }
        }
    }
}

void map_print(const BattleMap* map) {
    if (!map || !map->tiles) {
        printf("[地图] 空地图\n");
        return;
    }
    
    printf("[地图] %dx%d\n", map->width, map->height);
    
    const char* terrain_chars = " .~#=WRCV";
    
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            MapTile* tile = &map->tiles[y * map->width + x];
            byte idx = tile->terrain;
            if (idx >= TERRAIN_MAX) idx = 0;
            printf("%c", terrain_chars[idx]);
        }
        printf("\n");
    }
}
