#ifndef FD2_MAP_H
#define FD2_MAP_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_MAP_WIDTH 64
#define MAX_MAP_HEIGHT 64
#define MAX_TILES 256
#define MAX_LEVELS 100
#define TILE_SIZE 24

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

typedef enum {
    TERRAIN_PLAINS = 0,
    TERRAIN_FOREST = 1,
    TERRAIN_MOUNTAIN = 2,
    TERRAIN_WATER = 3,
    TERRAIN_WALL = 4,
    TERRAIN_ROAD = 5,
    TERRAIN_BRIDGE = 6,
    TERRAIN_CASTLE = 7,
    TERRAIN_VILLAGE = 8,
    TERRAIN_MAX
} TerrainType;

typedef enum {
    TILE_EMPTY = 0,
    TILE_SOLID = 1,
    TILE_WALKABLE = 2,
    TILE_FLYABLE = 3,
    TILE_WATER = 4,
    TILE_SPECIAL = 5
} TileFlag;

typedef struct {
    byte tile_id;
    byte terrain;
    byte height;
    byte flags;
    byte defense_bonus;
    byte attack_bonus;
    byte move_cost;
    byte padding[2];
} MapTile;

typedef struct {
    byte width;
    byte height;
    byte terrain_type;
    byte background_id;
    byte music_id;
    byte enemy_count;
    byte ally_count;
    byte player_count;
    byte padding;
    dword field_data_offset;
    dword shap_data_offset;
    dword bgm_offset;
} LevelHeader;

typedef struct {
    byte width;
    byte height;
    MapTile* tiles;
    LevelHeader* header;
    byte* raw_data;
    dword raw_size;
} BattleMap;

typedef struct {
    byte x;
    byte y;
    byte width;
    byte height;
} MapRect;

typedef struct {
    byte start_x;
    byte start_y;
    byte end_x;
    byte end_y;
    byte move_cost;
} PathNode;

typedef struct {
    PathNode* nodes;
    int node_count;
    int max_nodes;
} Path;

typedef struct {
    byte tile_index;
    byte sub_index;
    byte width;
    byte height;
    byte* data;
    dword size;
} TileGraphics;

extern const byte g_terrain_move_cost[TERRAIN_MAX];
extern const byte g_terrain_defense_bonus[TERRAIN_MAX];

int map_load_level(BattleMap* map, const byte* field_data, dword field_size, const LevelHeader* header);
void map_free(BattleMap* map);
MapTile* map_get_tile(BattleMap* map, byte x, byte y);
bool map_is_walkable(BattleMap* map, byte x, byte y);
bool map_is_in_bounds(BattleMap* map, byte x, byte y);
byte map_get_move_cost(BattleMap* map, byte x, byte y);
byte map_get_terrain_defense(BattleMap* map, byte x, byte y);

int map_find_path(BattleMap* map, byte start_x, byte start_y, byte end_x, byte end_y, Path* path, byte move_range);
void map_get_reachable_tiles(BattleMap* map, byte x, byte y, byte move_range, bool reachable[MAX_MAP_WIDTH][MAX_MAP_HEIGHT]);
void map_get_attackable_tiles(BattleMap* map, byte x, byte y, byte attack_range, bool attackable[MAX_MAP_WIDTH][MAX_MAP_HEIGHT]);

int map_save(const BattleMap* map, const char* filename);
int map_load(BattleMap* map, const char* filename);

void map_print(const BattleMap* map);

#endif
