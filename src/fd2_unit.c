/**
 * FD2 角色/单位系统实现
 * 基于IDA Pro逆向工程分析
 * 
 * 单位数据结构 (80字节):
 * - 偏移0-53: 属性数据
 * - 偏移54-79: 状态数据
 */

#include "../include/fd2_unit.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const JobBaseStats g_job_table[JOB_MAX] = {
    {100, 20, 10, 8, 5, 5, 8, 5, 5, 2, 2, 1, 1, 1, 1, 2, 4, 1, 0, 0, 1, 0, {0}},
    {120, 30, 15, 12, 5, 8, 6, 5, 6, 2, 3, 2, 1, 1, 1, 2, 5, 1, 1, 0, 2, 0, {0}},
    {80, 25, 12, 6, 8, 4, 10, 8, 4, 1, 2, 1, 1, 1, 0, 1, 4, 2, 0, 0, 3, 0, {0}},
    {70, 80, 8, 4, 15, 8, 6, 10, 3, 1, 1, 1, 2, 1, 0, 1, 3, 3, 0, 0, 4, 0, {0}},
    {80, 70, 6, 6, 12, 10, 6, 12, 3, 1, 1, 1, 2, 1, 0, 1, 3, 2, 0, 0, 4, 0, {0}},
    {75, 25, 10, 5, 6, 5, 12, 10, 4, 1, 2, 1, 1, 1, 0, 2, 5, 1, 0, 0, 1, 0, {0}},
    {110, 35, 18, 10, 8, 6, 8, 6, 5, 2, 3, 2, 1, 1, 0, 2, 5, 1, 0, 0, 2, 0, {0}},
    {85, 40, 14, 7, 10, 6, 14, 8, 5, 2, 2, 1, 2, 1, 0, 3, 6, 2, 0, 0, 2, 0, {0}},
    {130, 40, 16, 14, 6, 10, 7, 6, 6, 3, 3, 2, 1, 2, 1, 2, 6, 1, 0, 0, 2, 0, {0}},
    {90, 30, 14, 8, 10, 6, 8, 6, 4, 2, 2, 1, 1, 1, 0, 2, 4, 2, 0, 0, 3, 0, {0}},
    {140, 20, 20, 8, 4, 4, 10, 4, 6, 3, 4, 1, 1, 0, 0, 2, 4, 1, 0, 0, 1, 0, {0}},
    {65, 100, 6, 4, 18, 8, 6, 12, 3, 1, 1, 1, 3, 1, 0, 1, 3, 4, 0, 0, 4, 0, {0}},
    {85, 75, 6, 6, 14, 12, 6, 14, 3, 1, 1, 1, 2, 2, 0, 1, 3, 2, 0, 0, 4, 0, {0}},
    {80, 35, 16, 6, 8, 6, 16, 10, 5, 2, 3, 1, 2, 1, 0, 3, 6, 2, 0, 0, 2, 0, {0}},
    {105, 25, 14, 10, 6, 6, 8, 6, 5, 2, 3, 2, 1, 1, 0, 2, 5, 1, 0, 0, 2, 0, {0}}
};

const char* g_job_names[JOB_MAX] = {
    "士兵", "骑士", "弓箭手", "法师", "祭司", "盗贼", 
    "武士", "忍者", "龙骑士", "枪手", "狂战士", "巫师", 
    "牧师", "刺客", "枪兵"
};

void unit_init(Unit* u) {
    if (!u) return;
    memset(u, 0, sizeof(Unit));
    u->hp = 100;
    u->max_hp = 100;
    u->mp = 20;
    u->max_mp = 20;
    u->attack = 10;
    u->defense = 8;
    u->level = 1;
    u->team = TEAM_PLAYER;
    u->ai_type = AI_PLAYER_CONTROLLED;
}

void unit_array_init(UnitArray* ua) {
    if (!ua) return;
    memset(ua, 0, sizeof(UnitArray));
}

UnitArray* unit_array_load_from_save(const byte* save_data, byte unit_count) {
    if (!save_data || unit_count == 0 || unit_count > MAX_UNITS) {
        return NULL;
    }
    
    UnitArray* ua = (UnitArray*)malloc(sizeof(UnitArray));
    if (!ua) return NULL;
    
    unit_array_init(ua);
    ua->unit_count = unit_count;
    
    const byte* unit_data = save_data + 4771;
    
    for (int i = 0; i < unit_count && i < MAX_UNITS; i++) {
        const byte* src = unit_data + i * UNIT_SIZE;
        Unit* u = &ua->units[i];
        
        memcpy(u, src, sizeof(Unit));
    }
    
    return ua;
}

word unit_exp_for_level(word level) {
    if (level == 0) return 0;
    return level * 100 + level * level * 5;
}

void unit_level_up(Unit* u) {
    if (!u) return;
    
    word exp_needed = unit_exp_for_level(u->level);
    if (u->experience >= exp_needed) {
        u->level++;
        
        const JobBaseStats* job = &g_job_table[0];
        
        u->max_hp += job->growth_hp;
        u->max_mp += job->growth_mp;
        u->attack += job->growth_attack;
        u->defense += job->growth_defense;
        u->magic += job->growth_magic;
        u->resistance += job->growth_resistance;
        u->agility += job->growth_agility;
        u->luck += job->growth_luck;
        
        u->hp = u->max_hp;
        u->mp = u->max_mp;
        
        printf("[升级] 升到 %d 级\n", u->level);
    }
}

int unit_can_promote(const Unit* u) {
    if (!u) return 0;
    if (u->level < 10) return 0;
    return 1;
}

void unit_promote(Unit* u, byte new_job) {
    if (!u) return;
    if (new_job >= JOB_MAX) return;
    if (!unit_can_promote(u)) return;
    
    printf("[转职] 转职为 %s\n", g_job_names[new_job]);
}

const char* unit_get_job_name(byte job) {
    if (job >= JOB_MAX) return "未知";
    return g_job_names[job];
}

int unit_calculate_damage(const Unit* attacker, const Unit* defender) {
    if (!attacker || !defender) return 0;
    
    int base_damage = attacker->attack - defender->defense;
    if (base_damage < 1) base_damage = 1;
    
    int luck_mod = (attacker->luck - defender->luck) / 2;
    
    int variance = (rand() % 11) - 5;
    
    return base_damage + luck_mod + variance;
}

int unit_calculate_magic_damage(const Unit* attacker, const Unit* defender) {
    if (!attacker || !defender) return 0;
    
    int base_damage = attacker->magic - defender->resistance;
    if (base_damage < 1) base_damage = 1;
    
    int variance = (rand() % 11) - 5;
    
    return base_damage + variance;
}

void unit_take_damage(Unit* u, int damage) {
    if (!u) return;
    if (damage < 0) damage = 0;
    
    if (u->hp <= damage) {
        u->hp = 0;
        printf("[战斗] 被击败!\n");
    } else {
        u->hp -= damage;
        printf("[战斗] 受到 %d 伤害，剩余 %d HP\n", damage, u->hp);
    }
}

int unit_is_alive(const Unit* u) {
    return u && u->hp > 0;
}

void unit_move_to(Unit* u, byte x, byte y) {
    if (!u) return;
    if (!unit_is_alive(u)) return;
    
    u->x = x;
    u->y = y;
    u->has_moved = true;
    
    if (x > u->x) u->direction = DIR_RIGHT;
    else if (x < u->x) u->direction = DIR_LEFT;
    else if (y > u->y) u->direction = DIR_DOWN;
    else if (y < u->y) u->direction = DIR_UP;
}

void unit_attack(Unit* attacker, Unit* target) {
    if (!attacker || !target) return;
    if (!unit_is_alive(attacker) || !unit_is_alive(target)) return;
    
    int damage = unit_calculate_damage(attacker, target);
    unit_take_damage(target, damage);
    
    attacker->has_attacked = true;
}

Unit* unit_array_get_unit(UnitArray* ua, int index) {
    if (!ua) return NULL;
    if (index < 0 || index >= ua->unit_count) return NULL;
    return &ua->units[index];
}

int unit_array_find_by_position(UnitArray* ua, byte x, byte y) {
    if (!ua) return -1;
    for (int i = 0; i < ua->unit_count; i++) {
        if (ua->units[i].x == x && ua->units[i].y == y && unit_is_alive(&ua->units[i])) {
            return i;
        }
    }
    return -1;
}

int unit_array_find_enemy(UnitArray* ua, int exclude_index) {
    if (!ua) return -1;
    for (int i = 0; i < ua->unit_count; i++) {
        if (i == exclude_index) continue;
        Unit* u = &ua->units[i];
        if (u->team != ua->units[exclude_index].team && unit_is_alive(u)) {
            return i;
        }
    }
    return -1;
}

int unit_array_count_alive(UnitArray* ua, byte team) {
    if (!ua) return 0;
    int count = 0;
    for (int i = 0; i < ua->unit_count; i++) {
        if (ua->units[i].team == team && unit_is_alive(&ua->units[i])) {
            count++;
        }
    }
    return count;
}

void unit_raw_to_unit(const UnitRaw* raw, Unit* u) {
    if (!raw || !u) return;
    memset(u, 0, sizeof(Unit));
    
    u->hp = raw->hp;
    u->max_hp = raw->max_hp;
    u->mp = raw->mp;
    u->max_mp = raw->max_mp;
    u->attack = raw->attack;
    u->defense = raw->defense;
    u->magic = raw->magic;
    u->resistance = raw->resistance;
    u->agility = raw->agility;
    u->luck = raw->luck;
    u->move_range = raw->move_range;
    u->attack_range = raw->attack_range;
    u->experience = raw->experience;
    u->level = raw->level;
    
    u->x = (byte)(raw->hp & 0xFF);  
    u->y = (byte)(raw->max_hp & 0xFF);
    u->direction = raw->direction;
    u->status_flags = raw->status_flags;
    u->team = raw->team;
    u->ai_type = raw->ai_type;
    u->select_flags = raw->select_flags;
    u->action_points = raw->action_points;
    u->attack_count = raw->attack_count;
    u->move_cost = raw->move_cost;
}

void unit_to_raw(const Unit* u, UnitRaw* raw) {
    if (!u || !raw) return;
    memset(raw, 0, sizeof(UnitRaw));
    
    raw->hp = u->hp;
    raw->max_hp = u->max_hp;
    raw->mp = u->mp;
    raw->max_mp = u->max_mp;
    raw->attack = u->attack;
    raw->defense = u->defense;
    raw->magic = u->magic;
    raw->resistance = u->resistance;
    raw->agility = u->agility;
    raw->luck = u->luck;
    raw->move_range = u->move_range;
    raw->attack_range = u->attack_range;
    raw->experience = u->experience;
    raw->level = u->level;
    
    raw->direction = u->direction;
    raw->status_flags = u->status_flags;
    raw->team = u->team;
    raw->ai_type = u->ai_type;
    raw->select_flags = u->select_flags;
    raw->action_points = u->action_points;
    raw->attack_count = u->attack_count;
    raw->move_cost = u->move_cost;
}
