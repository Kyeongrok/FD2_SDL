#ifndef FD2_MOVEMENT_H
#define FD2_MOVEMENT_H

#include "fd2_unit.h"
#include "fd2_resources.h"
#include <stdbool.h>

#define MAX_PATH_LENGTH 64

typedef struct {
    byte x;
    byte y;
    short g_cost;
    short h_cost;
    short f_cost;
    byte parent_x;
    byte parent_y;
    bool in_open_set;
    bool in_closed_set;
    bool passable;
} PathNode;

typedef struct {
    byte path_x[MAX_PATH_LENGTH];
    byte path_y[MAX_PATH_LENGTH];
    byte path_length;
    byte current_index;
} UnitPath;

#define MAX_MAP_DIM 64

typedef struct {
    byte move_range;
    bool reachable[MAX_MAP_DIM][MAX_MAP_DIM];
    byte nearest_x;
    byte nearest_y;
    int reachable_count;
} MoveRange;

int movement_calculate_move_range(const Unit* unit, const MapData* map, MoveRange* range);
int movement_is_tile_passable(const MapData* map, byte x, byte y);
int movement_find_path(const MapData* map, byte start_x, byte start_y, 
                       byte target_x, byte target_y, UnitPath* path);
void movement_execute_path(Unit* unit, UnitPath* path);
bool movement_has_path(const UnitPath* path);
byte movement_get_path_length(const UnitPath* path);
void movement_clear_path(UnitPath* path);

#endif
