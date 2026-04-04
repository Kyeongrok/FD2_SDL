#ifndef FD2_UNIT_H
#define FD2_UNIT_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_UNITS 96
#define MAX_UNIT_NAME_LEN 16
#define UNIT_SIZE 80

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

typedef enum {
    JOB_SOLDIER = 0,
    JOB_KNIGHT = 1,
    JOB_ARCHER = 2,
    JOB_MAGE = 3,
    JOB_PRIEST = 4,
    JOB_THIEF = 5,
    JOB_SAMURAI = 6,
    JOB_NINJA = 7,
    JOB_DRAGON_KNIGHT = 8,
    JOB_GUNNER = 9,
    JOB_BERSERKER = 10,
    JOB_WIZARD = 11,
    JOB_CLERIC = 12,
    JOB_ASSASSIN = 13,
    JOB_LANCER = 14,
    JOB_MAX
} JobType;

typedef enum {
    TEAM_PLAYER = 1,
    TEAM_ENEMY = 2,
    TEAM_ALLY = 3,
    TEAM_NEUTRAL = 4
} TeamType;

typedef enum {
    AI_NONE = 0,
    AI_PLAYER_CONTROLLED = 1,
    AI_ENEMY = 2,
    AI_ALLY = 3,
    AI_NEUTRAL = 4
} AIType;

typedef enum {
    DIR_UP = 0,
    DIR_RIGHT = 1,
    DIR_DOWN = 2,
    DIR_LEFT = 3
} Direction;

typedef enum {
    STATUS_NORMAL = 0,
    STATUS_POISONED = 1,
    STATUS_STUNNED = 2,
    STATUS_SLEEPING = 4,
    STATUS_SILENCED = 8,
    STATUS_DEAD = 128
} StatusFlags;

typedef enum {
    STATE_IDLE = 0,
    STATE_MOVING = 1,
    STATE_ATTACKING = 2,
    STATE_DEFENDING = 3,
    STATE_DEAD = 4,
    STATE_STUNNED = 5,
    STATE_WAITING = 6,
    STATE_SELECTED = 7
} UnitState;

#pragma pack(push, 1)
typedef struct {
    word hp;
    word max_hp;
    word mp;
    word max_mp;
    word attack;
    word defense;
    word magic;
    word resistance;
    word agility;
    word luck;
    word move_range;
    word attack_range;
    word experience;
    word level;
    word padding1;
    byte direction;
    byte status_flags;
    byte team;
    byte ai_type;
    byte padding2;
    byte padding3;
    byte unknown1;
    byte unknown2;
    byte unknown3;
    byte unknown4;
    byte select_flags;
    byte action_points;
    byte attack_count;
    byte move_cost;
    byte field_28;
    byte field_29;
    byte field_2A;
    byte field_2B;
    byte field_2C;
    byte field_2D;
    byte field_2E;
    byte field_2F;
    byte field_30;
    byte field_31;
    byte field_32;
    byte field_33;
    byte field_34;
    byte field_35;
    byte field_36;
    byte field_37;
    byte field_38;
    byte field_39;
    byte field_3A;
    byte field_3B;
    byte field_3C;
    byte field_3D;
    byte field_3E;
    byte field_3F;
    byte field_40;
    byte field_41;
    byte field_42;
    byte field_43;
    byte field_44;
    byte field_45;
    byte field_46;
    byte field_47;
    byte field_48;
    byte field_49;
    byte field_4A;
    byte field_4B;
    byte field_4C;
    byte field_4D;
    byte field_4E;
    byte field_4F;
} UnitRaw;
#pragma pack(pop)

typedef struct {
    char name[MAX_UNIT_NAME_LEN];
    byte job;
    byte face_id;
    byte icon_id;
    byte rank;
    byte status_flags;
    byte element;
    byte can_promote;
    byte padding[3];
} UnitInfo;

typedef struct {
    word hp;
    word max_hp;
    word mp;
    word max_mp;
    word attack;
    word defense;
    word magic;
    word resistance;
    word agility;
    word luck;
    word move_range;
    word attack_range;
    word experience;
    word level;
    byte x;
    byte y;
    byte direction;
    byte status_flags;
    byte team;
    byte ai_type;
    byte select_flags;
    byte action_points;
    word attack_count;
    word move_cost;
    bool has_moved;
    bool has_attacked;
    bool is_selected;
} Unit;

typedef struct {
    Unit units[MAX_UNITS];
    byte unit_count;
    byte player_team_count;
    byte enemy_team_count;
    byte ally_team_count;
    byte padding[4];
    dword battle_flags;
} UnitArray;

typedef struct {
    dword level_id;
    char name[32];
    byte width;
    byte height;
    byte terrain_type;
    byte padding[3];
    dword field_data_offset;
    dword shap_data_offset;
    byte enemy_count;
    byte ally_count;
    byte background_id;
    byte music_id;
} LevelInfo;

typedef struct {
    word base_hp;
    word base_mp;
    word base_attack;
    word base_defense;
    word base_magic;
    word base_resistance;
    word base_agility;
    word base_luck;
    
    word growth_hp;
    word growth_mp;
    word growth_attack;
    word growth_defense;
    word growth_magic;
    word growth_resistance;
    word growth_agility;
    word growth_luck;
    
    word move_range;
    word attack_range;
    byte weapon_type;
    byte armor_type;
    byte promotion_job;
    byte padding[3];
} JobBaseStats;

typedef struct {
    byte x;
    byte y;
    byte terrain;
    byte height;
} MapTile;

typedef struct {
    byte width;
    byte height;
    MapTile* tiles;
} BattleMap;

extern const JobBaseStats g_job_table[JOB_MAX];
extern const char* g_job_names[JOB_MAX];

void unit_init(Unit* u);
void unit_array_init(UnitArray* ua);
UnitArray* unit_array_load_from_save(const byte* save_data, byte unit_count);
word unit_exp_for_level(word level);
void unit_level_up(Unit* u);
int unit_can_promote(const Unit* u);
void unit_promote(Unit* u, byte new_job);
const char* unit_get_job_name(byte job);
int unit_calculate_damage(const Unit* attacker, const Unit* defender);
int unit_calculate_magic_damage(const Unit* attacker, const Unit* defender);
void unit_take_damage(Unit* u, int damage);
int unit_is_alive(const Unit* u);
void unit_move_to(Unit* u, byte x, byte y);
void unit_attack(Unit* attacker, Unit* target);

Unit* unit_array_get_unit(UnitArray* ua, int index);
int unit_array_find_by_position(UnitArray* ua, byte x, byte y);
int unit_array_find_enemy(UnitArray* ua, int exclude_index);
int unit_array_count_alive(UnitArray* ua, byte team);

void unit_raw_to_unit(const UnitRaw* raw, Unit* u);
void unit_to_raw(const Unit* u, UnitRaw* raw);

#endif
