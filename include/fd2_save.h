#ifndef FD2_SAVE_H
#define FD2_SAVE_H

#include "fd2_unit.h"
#include <stdint.h>

#define SAVE_FILE_SIZE 22987

#define SAVE_OFFSET_UNIT_DATA 4771
#define SAVE_OFFSET_UNIT_COUNT 12484
#define SAVE_OFFSET_STATE 12485
#define SAVE_OFFSET_UNKNOWN 12486
#define SAVE_OFFSET_CHECKSUM 12482
#define SAVE_OFFSET_PALETTE 2211

typedef struct {
    byte data[SAVE_FILE_SIZE];
} SaveFile;

typedef struct {
    word current_level;
    word chapter;
    byte padding[2];
    dword game_time;
    dword total_kills;
    dword total_gold;
    byte difficulty;
    byte game_mode;
    byte padding2[6];
} GameHeader;

typedef struct {
    byte chapter;
    byte mission;
    byte padding[2];
    byte player_units[12];
    byte ally_units[8];
    byte enemy_units[20];
    word gold;
    word padding2;
} MissionData;

int save_check_checksum(const byte* data, int size);
dword save_calculate_checksum(const byte* data, int size);
int save_load(SaveFile* save, const char* filename);
int save_save(const SaveFile* save, const char* filename);

GameHeader* save_get_header(const SaveFile* save);
MissionData* save_get_mission(const SaveFile* save);
byte* save_get_unit_data(const SaveFile* save);
byte save_get_unit_count(const SaveFile* save);
byte save_get_game_state(const SaveFile* save);

#endif
