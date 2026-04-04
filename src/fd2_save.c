/**
 * FD2 存档系统实现
 * 基于IDA Pro逆向工程分析
 * 
 * 存档结构 (FD2.SAV - 22987字节):
 * Offset 0x0000 (0):     主游戏数据 (2211字节)
 * Offset 0x0897 (2211): 调色板数据 (2560字节)
 * Offset 0x124B (4771): 单位数据开始 (80字节/单位)
 * Offset 0x1244 (12484): 单位数量 (1字节)
 * Offset 0x1245 (12485): 游戏状态 (1字节)
 * Offset 0x1246 (12486): 未知 (1字节)
 * Offset 0x1247 (12487): 校验和高字节 (2字节)
 * Offset 0x1249 (12489): 校验和低字节
 */

#include "../include/fd2_save.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static dword g_checksum_seed = 0x12345678;

static dword simple_hash(const byte* data, int len) {
    dword hash = g_checksum_seed;
    for (int i = 0; i < len; i++) {
        hash = hash * 33 + data[i];
    }
    return hash;
}

/**
 * 计算校验和 (基于sub_4DF09)
 * IDA分析: 对数据逐字节累加
 */
dword save_calculate_checksum(const byte* data, int size) {
    if (!data || size <= 0) return 0;
    
    dword checksum = 0;
    for (int i = 0; i < size - 4; i++) {
        checksum += data[i];
    }
    
    return checksum;
}

/**
 * 验证校验和 (基于sub_4DF28 + sub_4DF09)
 */
int save_check_checksum(const byte* data, int size) {
    if (!data || size <= 4) return 0;
    
    dword stored = *(dword*)(data + size - 4);
    dword computed = save_calculate_checksum(data, size);
    
    return (stored == computed) ? 1 : 0;
}

/**
 * 加载存档文件
 */
int save_load(SaveFile* save, const char* filename) {
    if (!save || !filename) return 0;
    
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("[存档] 无法打开文件: %s\n", filename);
        return 0;
    }
    
    size_t read = fread(save->data, 1, SAVE_FILE_SIZE, fp);
    fclose(fp);
    
    if (read != SAVE_FILE_SIZE) {
        printf("[存档] 文件大小不正确: %zu / %d\n", read, SAVE_FILE_SIZE);
        return 0;
    }
    
    if (!save_check_checksum(save->data, SAVE_FILE_SIZE)) {
        printf("[存档] 校验和验证失败\n");
        return 0;
    }
    
    printf("[存档] 成功加载: %s\n", filename);
    return 1;
}

/**
 * 保存存档文件
 */
int save_save(const SaveFile* save, const char* filename) {
    if (!save || !filename) return 0;
    
    SaveFile temp;
    memcpy(&temp, save, sizeof(SaveFile));
    
    dword checksum = save_calculate_checksum(temp.data, SAVE_FILE_SIZE - 4);
    *(dword*)(temp.data + SAVE_FILE_SIZE - 4) = checksum;
    
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        printf("[存档] 无法创建文件: %s\n", filename);
        return 0;
    }
    
    size_t written = fwrite(temp.data, 1, SAVE_FILE_SIZE, fp);
    fclose(fp);
    
    if (written != SAVE_FILE_SIZE) {
        printf("[存档] 写入失败\n");
        return 0;
    }
    
    printf("[存档] 成功保存: %s\n", filename);
    return 1;
}

/**
 * 获取存档头数据
 */
GameHeader* save_get_header(const SaveFile* save) {
    if (!save) return NULL;
    return (GameHeader*)save->data;
}

/**
 * 获取任务数据
 */
MissionData* save_get_mission(const SaveFile* save) {
    if (!save) return NULL;
    return (MissionData*)(save->data + 100);
}

/**
 * 获取单位数据起始位置
 */
byte* save_get_unit_data(const SaveFile* save) {
    if (!save) return NULL;
    return (byte*)save->data + SAVE_OFFSET_UNIT_DATA;
}

/**
 * 获取单位数量
 */
byte save_get_unit_count(const SaveFile* save) {
    if (!save) return 0;
    return save->data[SAVE_OFFSET_UNIT_COUNT];
}

/**
 * 获取游戏状态
 */
byte save_get_game_state(const SaveFile* save) {
    if (!save) return 0;
    return save->data[SAVE_OFFSET_STATE];
}

/**
 * 创建新存档
 */
void save_create_new(SaveFile* save) {
    if (!save) return;
    memset(save, 0, sizeof(SaveFile));
    
    GameHeader* header = save_get_header(save);
    header->current_level = 1;
    header->chapter = 1;
    header->difficulty = 1;
    header->game_mode = 0;
    header->total_gold = 1000;
}

/**
 * 获取存档摘要信息
 */
void save_print_info(const SaveFile* save) {
    if (!save) return;
    
    GameHeader* header = save_get_header(save);
    byte unit_count = save_get_unit_count(save);
    byte state = save_get_game_state(save);
    
    printf("=== 存档信息 ===\n");
    printf("关卡: %d\n", header->current_level);
    printf("章节: %d\n", header->chapter);
    printf("金币: %d\n", header->total_gold);
    printf("难度: %d\n", header->difficulty);
    printf("单位数: %d\n", unit_count);
    printf("游戏状态: %d\n", state);
}
