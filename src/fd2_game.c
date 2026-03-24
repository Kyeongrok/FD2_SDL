/**
 * FD2.exe 完整重新实现
 * 基于IDA Pro MCP服务器对fd2.exe的逆向工程分析
 * 
 * 游戏: Puzzle Beauty (美丽拼图)
 * 1993年经典DOS拼图游戏的重现代替品
 */

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef USE_SDL
#include "fd2_sdl_renderer.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

// 基本类型
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

// 游戏常量
#define GAME_TITLE "Puzzle Beauty"
#define GAME_VERSION "1.0"
#define MAX_SPRITES 256
#define MAX_LEVELS 100
#define FRAME_DELAY 50
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_SIZE 256

// 游戏状态
typedef enum {
    STATE_INIT = 0,
    STATE_TITLE = 1,
    STATE_MENU = 2,
    STATE_LEVEL_SELECT = 3,
    STATE_PLAYING = 4,
    STATE_PAUSED = 5,
    STATE_WIN = 6,
    STATE_QUIT = 99
} GameStateID;

// DAT文件句柄
typedef struct {
    byte* data;
    dword size;
    int resource_count;
    int max_resources;
    dword* starts;
    dword* ends;
} DatHandle;

// 精灵信息
typedef struct {
    int width;
    int height;
    byte* data;
    int size;
} Sprite;

// 资源缓存
typedef struct {
    byte* palette;
    byte* sprites_22;
    byte* sprites_32;
    int sprite_count_22;
    int sprite_count_32;
    
    DatHandle fdother;
    DatHandle fdfield;
    DatHandle fdshap;
    DatHandle fdtxt;
    
    byte* field_data;       // RLE解压后的场地数据
    int field_width;
    int field_height;
    int field_stride;
    
    byte* level_header;     // 当前关卡的FDFIELD资源0数据
    byte* level_shapes;     // 当前关卡的FDSHAP形状数据
    int level_shape_idx;    // 形状索引
    int piece_count;        // 拼图块数量
} ResourceCache;

// 游戏状态机
typedef struct {
    GameStateID current_state;
    GameStateID next_state;
    int current_level;
    int score;
    int moves;
    int selected_piece;
    int highlight_x;
    int highlight_y;
    bool running;
    int frame_count;
    uint32_t last_frame_time;
} GameMachine;

// 渲染器状态 (用于SDL模式)
typedef struct {
    byte palette[PALETTE_SIZE][3];
    byte screen_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    int anim_frame;
    int bg_color;
    bool initialized;
} RenderState;

// 前向声明
static bool parse_dat_handle(DatHandle* dh);
static byte* get_dat_resource(DatHandle* dh, int index, dword* out_size);
static int decompress_rle_simple(byte* src, int src_size, byte* dst, int dst_stride);
static bool load_level(int level);
static void apply_palette(void);
static void clear_screen(byte color);
static void present(void);
static void delay_ms(uint32_t ms);
static int run_ani_command(int cmd, byte* data, int pos_in_block, int block_size);
static Sprite get_sprite_22(int idx);

static int load_ani_cached(int resource_index);
static void update_ani_frame(void);

// ANI动画系统
static byte ani_palette_buf[768];
static byte ani_screen_buf[64000];
static byte* g_ani_cache = NULL;
static int g_ani_cache_size = 0;
static int g_ani_block_count = 0;
static int g_ani_res_idx = 0;
static int g_ani_frame = 0;

typedef struct {
    int offset;
    int size;
    int cmd_count;
} ANIBlockInfo;

static ANIBlockInfo* g_ani_blocks = NULL;
static int g_ani_max_blocks = 0;

// ANI命令处理器 - 基于IDA Pro逆向工程
static int do_h0(byte* d) { byte b=d[0]; dword v=(b<<16)|(b<<8)|b; dword* p=(dword*)ani_palette_buf; for(int i=0;i<96;i++)p[i]=v; return 1; }
static int do_h1(byte* d, int sz) { if(sz>768)sz=768; for(int i=0;i<sz;i++)ani_palette_buf[i]=d[i]; return sz; }
static int do_h2(byte* d, int sz) {
    byte* dst=ani_palette_buf; int filled=0,pos=0;
    while(filled<768&&pos<sz){
        byte b=d[pos++];
        if((b&0xC0)==0xC0){
            int run=b&0x3F;
            byte v=d[pos++];
            int count=(run>>1)+(run&1);
            for(int i=0;i<count&&filled<768;i++){dst[filled++]=v;}
        }else{
            dst[filled++]=b;
        }
    }
    return pos;
}
static int do_h3(byte* d, int sz) {
    int pos=0; if(pos>=sz)return 0;
    int count=d[pos++]; byte* dst=ani_palette_buf;
    for(int i=0;i<count&&pos+2<=sz;i++){
        int off=d[pos++];
        int cp=d[pos++];
        int actual_off=off*3;
        int actual_cp=(cp*3)>>1;
        if(actual_off>=0&&actual_cp>0&&actual_off+actual_cp<=768&&pos+actual_cp<=sz){
            memcpy(dst+actual_off,d+pos,actual_cp);
        }
        pos+=actual_cp;
    }
    return pos;
}
static int do_h4(byte* d) { byte b=d[0]; dword v=(b<<16)|(b<<8)|b; dword* p=(dword*)ani_screen_buf; for(int i=0;i<10000;i++)p[i]=v; for(int i=40000;i<64000;i++)ani_screen_buf[i]=b; return 1; }
static int do_h5(byte* d) { memcpy(ani_screen_buf,d,64000); return 64000; }
static int do_h6(byte* d, int sz) {
    byte* dst=ani_screen_buf; int filled=0,pos=0;
    while(filled<64000&&pos<sz){
        byte b=d[pos++];
        if((b&0xC0)==0xC0){
            int run=b&0x3F;
            byte v=d[pos++];
            int words=(run>>1);
            int bytes=run&1;
            for(int i=0;i<words&&filled<64000;i++){dst[filled++]=v;if(filled<64000)dst[filled++]=v;}
            for(int i=0;i<bytes&&filled<64000;i++)dst[filled++]=v;
        }else{
            dst[filled++]=b;
        }
    }
    return pos;
}
static int do_h7(byte* d, int sz) {
    int pos=0; if(pos>=sz)return 0;
    int count_lo=d[pos++],count_hi=d[pos++];
    int count=count_lo|(count_hi<<8);
    for(int i=0;i<count&&pos+3<=sz;i++){
        int off_lo=d[pos++],off_hi=d[pos++];
        int run=d[pos++];
        int val=d[pos++];
        int off=off_lo|(off_hi<<8);
        if(off>=0&&off<64000){
            int words=run>>1;
            int bytes=run&1;
            for(int j=0;j<words&&off<64000;j++){
                ani_screen_buf[off++]=val;
                if(off<64000)ani_screen_buf[off++]=val;
            }
            for(int j=0;j<bytes&&off<64000;j++)ani_screen_buf[off++]=val;
        }
    }
    return pos;
}
static int do_h8(byte* d, int sz) {
    int pos=0; if(pos>=sz)return 0;
    int count_lo=d[pos++],count_hi=d[pos++];
    int count=count_lo|(count_hi<<8);
    for(int i=0;i<count&&pos+2<=sz;i++){
        int off_lo=d[pos++],off_hi=d[pos++];
        int off=off_lo|(off_hi<<8);
        byte v=d[pos++];
        if(off>=0&&off<64000)ani_screen_buf[off]=v;
    }
    return pos;
}
static int do_h9(byte* d, int sz) {
    int pos=0; if(pos>=sz)return 0;
    int count=d[pos++];
    for(int i=0;i<count&&pos+4<=sz;i++){
        int off_lo=d[pos++],off_hi=d[pos++];
        int stride_lo=d[pos++],stride_hi=d[pos++];
        int off=off_lo|(off_hi<<8);
        int stride=stride_lo|(stride_hi<<8);
        if(off>=0&&stride>0&&off+stride<=64000&&pos+stride<=sz){memcpy(ani_screen_buf+off,d+pos,stride);}
        pos+=stride;
    }
    return pos;
}

static int run_ani_command(int cmd, byte* data, int pos_in_block, int block_size) {
    int remaining = block_size - pos_in_block;
    switch(cmd) {
        case 0: return do_h0(data+pos_in_block);
        case 1: return do_h1(data+pos_in_block, remaining);
        case 2: return do_h2(data+pos_in_block, remaining);
        case 3: return do_h3(data+pos_in_block, remaining);
        case 4: return do_h4(data+pos_in_block);
        case 5: return do_h5(data+pos_in_block);
        case 6: return do_h6(data+pos_in_block, remaining);
        case 7: return do_h7(data+pos_in_block, remaining);
        case 8: return do_h8(data+pos_in_block, remaining);
        case 9: return do_h9(data+pos_in_block, remaining);
        default: return 0;
    }
}

static int load_ani_cached(int resource_index) {
    FILE* fp = fopen("ANI.DAT", "rb");
    if (!fp) return -1;
    
    fseek(fp, 4 * resource_index + 6, SEEK_SET);
    dword offset;
    if (fread(&offset, 4, 1, fp) != 1) { fclose(fp); return -1; }
    
    fseek(fp, offset, SEEK_SET);
    byte header[167];
    if (fread(header, 1, 167, fp) != 167) { fclose(fp); return -1; }
    
    word block_count = *(word*)(header + 165);
    if (block_count <= 0 || block_count > 1000) { fclose(fp); return -1; }
    
    if (g_ani_blocks && g_ani_block_count > 0) {
        fclose(fp);
        return 0;
    }
    
    free(g_ani_blocks);
    g_ani_blocks = (ANIBlockInfo*)malloc(block_count * sizeof(ANIBlockInfo));
    if (!g_ani_blocks) { fclose(fp); return -1; }
    g_ani_max_blocks = block_count;
    
    int file_pos = offset + 165;
    int cached_size = 0;
    
    for (int i = 0; i < block_count; i++) {
        byte block_header[6];
        if (fseek(fp, file_pos, SEEK_SET) != 0 || fread(block_header, 1, 6, fp) != 6) break;
        
        word size = *(word*)(block_header + 0);
        word cmd_count = *(word*)(block_header + 2);
        
        g_ani_blocks[i].offset = file_pos + 6;
        g_ani_blocks[i].size = size;
        g_ani_blocks[i].cmd_count = cmd_count;
        
        if (size > 0 && size <= 64000) {
            cached_size += size;
        } else {
            g_ani_blocks[i].size = 0;
            g_ani_blocks[i].cmd_count = 0;
        }
        file_pos += 6 + size;
    }
    
    free(g_ani_cache);
    g_ani_cache = (byte*)malloc(cached_size);
    if (!g_ani_cache) { free(g_ani_blocks); fclose(fp); return -1; }
    g_ani_cache_size = cached_size;
    
    int cache_offset = 0;
    for (int i = 0; i < block_count; i++) {
        int size = g_ani_blocks[i].size;
        if (size > 0) {
            fseek(fp, g_ani_blocks[i].offset, SEEK_SET);
            if (fread(g_ani_cache + cache_offset, 1, size, fp) == (size_t)size) {
                g_ani_blocks[i].offset = cache_offset;
                cache_offset += size;
            } else {
                g_ani_blocks[i].size = 0;
            }
        }
    }
    
    g_ani_block_count = block_count;
    fclose(fp);
    return 0;
}

static int decode_ani_block(int block_index) {
    FILE* fp = fopen("ANI.DAT", "rb");
    if (!fp) return -1;
    
    fseek(fp, 4 * g_ani_res_idx + 6, SEEK_SET);
    dword offset;
    if (fread(&offset, 4, 1, fp) != 1) { fclose(fp); return -1; }
    
    fseek(fp, offset, SEEK_SET);
    byte header[173];
    if (fread(header, 1, 173, fp) != 173) { fclose(fp); return -1; }
    
    word block_count = *(word*)(header + 165);
    if (block_index < 0 || block_index >= block_count) { fclose(fp); return -1; }
    
    static byte block_data[64000];
    int frames_decoded = 0;
    
    for (int i = 0; i < block_count; i++) {
        byte block_header[8];
        if (fread(block_header, 1, 8, fp) != 8) break;
        
        word size = *(word*)(block_header + 0);
        word cmd_count = *(word*)(block_header + 2);
        
        if (fseek(fp, size, SEEK_CUR) != 0) { fclose(fp); return -1; }
        
        if (size == 0 || cmd_count == 0) continue;
        
        if (frames_decoded == block_index) {
            fseek(fp, -size, SEEK_CUR);
            if (fread(block_data, 1, size, fp) != (size_t)size) { fclose(fp); return -1; }
            
            memset(ani_screen_buf, 0, 64000);
            memset(ani_palette_buf, 0, 768);
            
            int pos = 0;
            for (int c = 0; c < cmd_count && pos < size; c++) {
                byte cmd = block_data[pos++];
                if (cmd < 10) {
                    pos += run_ani_command(cmd, block_data, pos, size);
                }
            }
            fclose(fp);
            return 1;
        }
        
        frames_decoded++;
    }
    
    fclose(fp);
    return 0;
}

static void update_ani_frame() {
    if (g_ani_block_count <= 0) {
        if (load_ani_cached(g_ani_res_idx) != 0) return;
    }
    int frame_idx = g_ani_frame % g_ani_block_count;
    if (frame_idx < 0 || frame_idx >= g_ani_block_count) return;
    decode_ani_block(frame_idx);
}

// 全局状态
static GameMachine g_machine;
static RenderState g_render;
static ResourceCache g_resources;
static bool g_sdl_active = false;

// 启动动画状态
typedef struct {
    int phase;
    int frame_count;
    int bar_offset;
    int bar_loaded;
    byte* bar_data;
    int fade_level;
    int current_res;
    bool complete;
    DatHandle figani;  // FIGANI.DAT句柄
} StartupAnimState;

static StartupAnimState g_startup = {0};



// 从FDOTHER.DAT绘制资源到屏幕 (可能需要RLE解压)


// 设置游戏调色板 (从FDOTHER.DAT资源获取)
static void set_game_palette(int res_idx) {
    if (!g_resources.fdother.data) return;
    dword size;
    byte* pal = get_dat_resource(&g_resources.fdother, res_idx, &size);
    if (pal && size >= 768) {
        memcpy(g_resources.palette, pal, 768);
        apply_palette();
    }
}

// 播放ANI.DAT指定资源
static int play_ani_resource(int res_idx, int frame_delay, int wait_key) {
    FILE* fp = fopen("ANI.DAT", "rb");
    if (!fp) return -1;
    
    fseek(fp, 4 * res_idx + 6, SEEK_SET);
    dword offset;
    if (fread(&offset, 4, 1, fp) != 1) { fclose(fp); return -1; }
    
    fseek(fp, offset, SEEK_SET);
    byte header[173];
    if (fread(header, 1, 173, fp) != 173) { fclose(fp); return -1; }
    
    word block_count = *(word*)(header + 165);
    if (block_count <= 0) { fclose(fp); return -1; }
    
    printf("[启动] 播放ANI.DAT资源%d (%d帧, %dms/帧)\n", res_idx, block_count, frame_delay);
    
    for (int i = 0; i < block_count; i++) {
        byte block_header[8];
        if (fread(block_header, 1, 8, fp) != 8) break;
        
        word size = *(word*)(block_header + 0);
        word cmd_count = *(word*)(block_header + 2);
        
        if (size == 0 || cmd_count == 0) continue;
        
        byte* block_data = (byte*)malloc(size);
        if (!block_data) continue;
        
        if (fread(block_data, 1, size, fp) != (size_t)size) {
            free(block_data);
            continue;
        }
        
        memset(ani_screen_buf, 0, 64000);
        memset(ani_palette_buf, 0, 768);
        
        int pos = 0;
        for (int c = 0; c < cmd_count && pos < size; c++) {
            byte cmd = block_data[pos++];
            if (cmd < 10) {
                pos += run_ani_command(cmd, block_data, pos, size);
            }
        }
        
        memcpy(g_render.screen_buffer, ani_screen_buf, 64000);
        
        if (g_resources.palette) {
            memcpy(g_resources.palette, ani_palette_buf, 768);
            apply_palette();
        }
        
        delay_ms(frame_delay);
        
        if (wait_key) {
#ifdef USE_SDL
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                    free(block_data);
                    fclose(fp);
                    return 1;
                }
            }
#endif
        }
        
        free(block_data);
    }
    
    fclose(fp);
    return 0;
}



// 淡入淡出效果
static void fade_effect(int start, int end, int frame_delay) {
    for (int level = start; level >= end; level--) {
        g_startup.fade_level = level;
        delay_ms(frame_delay);
    }
    g_startup.fade_level = end;
}

// 解压FDOTHER.DAT资源到屏幕缓冲区 (320x200)
static void decompress_to_screen(int res_idx) {
    dword size;
    byte* data = get_dat_resource(&g_resources.fdother, res_idx, &size);
    if (!data) return;
    
    // 使用RLE解压函数 - 它会自动读取header中的count和stride
    memset(g_render.screen_buffer, 0, 64000);
    decompress_rle_simple(data, size, g_render.screen_buffer, 320);
}

// 设置调色板 (从FDOTHER.DAT资源加载256色调色板)
static void set_palette(int res_idx) {
    dword size;
    byte* pal = get_dat_resource(&g_resources.fdother, res_idx, &size);
    if (pal && size >= 768) {
        memcpy(g_resources.palette, pal, 768);
        apply_palette();
        printf("[调色板] 已设置 (资源%d, %d字节)\n", res_idx, size);
    } else {
        printf("[调色板] 警告: 资源%d大小=%d\n", res_idx, size ? size : 0);
    }
}

// 完整的启动序列
static void run_full_startup_sequence() {
    printf("[启动] 开始完整启动序列...\n");
    
    memset(&g_startup, 0, sizeof(StartupAnimState));
    
    // ===== Phase 0: 加载调色板 =====
    // 资源102是真正的调色板 (768字节, 6-bit值)
    printf("[启动] Phase 0: 加载调色板 (资源102)\n");
    set_palette(102);
    apply_palette();
    present();
    delay_ms(100);
    
    // ===== Phase 1: 显示资源74 + ANI动画 =====
    printf("[启动] Phase 1: 绘制FDOTHER资源74 + ANI.DAT资源0\n");
    clear_screen(0);
    decompress_to_screen(74);
    present();
    delay_ms(30);
    
    // 播放ANI动画
    g_ani_res_idx = 0;
    if (load_ani_cached(0) == 0 && g_ani_block_count > 0) {
        printf("[启动] 播放ANI动画: %d帧\n", g_ani_block_count);
        for (int i = 0; i < 30 && i < g_ani_block_count; i++) {
            if (decode_ani_block(i) > 0) {
                memcpy(g_render.screen_buffer, ani_screen_buf, 64000);
                present();
                delay_ms(80);
            }
        }
    }
    
    // ===== Phase 2: 资源99 + ANI.DAT资源3 =====
    printf("[启动] Phase 2: 绘制FDOTHER资源99 + ANI.DAT资源3\n");
    clear_screen(0);
    set_palette(99);
    present();
    delay_ms(100);
    
    play_ani_resource(3, 90, 1);
    
    clear_screen(0);
    set_palette(101);
    present();
    delay_ms(100);
    
    // ===== Phase 3: 条形动画 =====
    printf("[启动] Phase 3: 条形动画 (535帧)\n");
    
    // 分配条形动画缓冲区
    byte* bar_buf = (byte*)malloc(5 * 147 * 320);
    if (!bar_buf) { printf("[错误] bar_buf分配失败\n"); return; }
    memset(bar_buf, 0, 5 * 147 * 320);
    printf("[启动] bar_buf分配成功: %d字节\n", 5 * 147 * 320);
    
    // 加载条形帧 (FDOTHER资源69-73)
    for (int i = 0; i < 5; i++) {
        dword size;
        byte* data = get_dat_resource(&g_resources.fdother, 69 + i, &size);
        if (data) {
            printf("[启动] 解压条形帧%d: 大小=%d\n", i, size);
            byte* decoded = (byte*)malloc(320 * 200);  // Use full screen height
            if (decoded) {
                memset(decoded, 0, 320 * 200);
                decompress_rle_simple(data, size, decoded, 320);
                memcpy(bar_buf + i * 147 * 320, decoded, 147 * 320);
                // Debug: check first few bytes
                printf("[启动] bar_buf[%d]前16字节: %02x %02x %02x %02x...\n", 
                       i * 147 * 320, bar_buf[i * 147 * 320], bar_buf[i * 147 * 320 + 1],
                       bar_buf[i * 147 * 320 + 2], bar_buf[i * 147 * 320 + 3]);
                free(decoded);
            } else {
                printf("[错误] decoded分配失败\n");
            }
        } else {
            printf("[错误] 无法获取条形帧%d\n", i);
        }
    }
    
    // 条形动画循环 (从offset=535开始，逐渐向上滚动)
    printf("[启动] 开始条形动画循环\n");
    for (int offset = 535; offset >= 0; offset--) {
        clear_screen(0);
        
        // 绘制条形 - 从offset开始复制200行到屏幕顶部
        int src_y = offset;
        int dst_y = 0;
        while (src_y < 5 * 147 && dst_y < SCREEN_HEIGHT) {
            byte* src_row = bar_buf + src_y * SCREEN_WIDTH;
            byte* dst_row = g_render.screen_buffer + dst_y * SCREEN_WIDTH;
            memcpy(dst_row, src_row, SCREEN_WIDTH);
            src_y++;
            dst_y++;
        }
        
        // 在特定帧切换调色板（根据IDA分析）
        if (offset == 330 || offset == 210 || offset == 110 || offset == 25) {
            set_palette(102);
            present();
            delay_ms(30);
            set_palette(101);
        } else if (offset == 450) {
            decompress_to_screen(100);
        } else if (offset == 10) {
            decompress_to_screen(75);
        }
        
        present();
        delay_ms(16);  // ~60fps
        
#ifdef USE_SDL
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                free(bar_buf);
                g_startup.complete = true;
                return;
            }
        }
#endif
    }
    
    printf("[启动] 条形动画循环完成\n");
    
    // ===== Phase 4: 淡出 =====
    printf("[启动] Phase 4: 淡出效果\n");
    fade_effect(40, 0, 8);
    delay_ms(100);
    
    // ===== Phase 5: 关卡选择 =====
    printf("[启动] Phase 5: 关卡选择画面\n");
    clear_screen(0);
    set_palette(7);
    decompress_to_screen(8);
    present();
    delay_ms(100);
    
    play_ani_resource(1, 15, 1);
    
    clear_screen(0);
    decompress_to_screen(7);
    set_game_palette(101);
    present();
    
    fade_effect(0, 40, 8);
    
    // ===== Phase 6: 等待选择 =====
    printf("[启动] Phase 6: 等待玩家选择\n");
    
    printf("[启动] 启动序列完成\n");
    g_startup.complete = true;
}

// 初始化游戏机器
static void init_game_machine(GameMachine* m) {
    memset(m, 0, sizeof(GameMachine));
    m->current_state = STATE_INIT;
    m->next_state = STATE_TITLE;
    m->current_level = 1;
    m->score = 0;
    m->moves = 0;
    m->running = true;
    m->frame_count = 0;
    m->last_frame_time = 0;
}

// 加载资源
static bool load_resources() {
    // 加载FDOTHER.DAT
    printf("[资源] 加载FDOTHER.DAT...\n");
    FILE* fp = fopen("FDOTHER.DAT", "rb");
    if (!fp) { printf("[错误] 无法打开FDOTHER.DAT\n"); return false; }
    fseek(fp, 0, SEEK_END);
    g_resources.fdother.size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    g_resources.fdother.data = (byte*)malloc(g_resources.fdother.size);
    fread(g_resources.fdother.data, 1, g_resources.fdother.size, fp);
    fclose(fp);
    parse_dat_handle(&g_resources.fdother);
    printf("[资源] FDOTHER.DAT: %d个资源\n", g_resources.fdother.resource_count);
    
    // 资源0: 调色板 (768字节) - 需要复制出来
    if (g_resources.fdother.resource_count > 0) {
        dword size;
        byte* pal_src = get_dat_resource(&g_resources.fdother, 0, &size);
        if (pal_src && size >= 768) {
            g_resources.palette = (byte*)malloc(768);
            memcpy(g_resources.palette, pal_src, 768);
            printf("[资源] 调色板: %d字节\n", size);
        }
    }
    
    // 资源1: 22x22精灵
    if (g_resources.fdother.resource_count > 1) {
        dword size;
        byte* data = get_dat_resource(&g_resources.fdother, 1, &size);
        g_resources.sprites_22 = (byte*)malloc(size);
        memcpy(g_resources.sprites_22, data, size);
        g_resources.sprite_count_22 = size / (22 * 22);
        printf("[资源] 22x22精灵: %d个 (%d字节)\n", g_resources.sprite_count_22, size);
    }
    
    // 资源2: 32x32精灵
    if (g_resources.fdother.resource_count > 2) {
        dword size;
        byte* data = get_dat_resource(&g_resources.fdother, 2, &size);
        g_resources.sprites_32 = (byte*)malloc(size);
        memcpy(g_resources.sprites_32, data, size);
        g_resources.sprite_count_32 = size / (32 * 32);
        printf("[资源] 32x32精灵: %d个 (%d字节)\n", g_resources.sprite_count_32, size);
    }
    
    // 加载FDFIELD.DAT
    printf("[资源] 加载FDFIELD.DAT...\n");
    fp = fopen("FDFIELD.DAT", "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        g_resources.fdfield.size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        g_resources.fdfield.data = (byte*)malloc(g_resources.fdfield.size);
        fread(g_resources.fdfield.data, 1, g_resources.fdfield.size, fp);
        fclose(fp);
        parse_dat_handle(&g_resources.fdfield);
        printf("[资源] FDFIELD.DAT: %d个资源\n", g_resources.fdfield.resource_count);
    }
    
    // 加载FDSHAP.DAT
    printf("[资源] 加载FDSHAP.DAT...\n");
    fp = fopen("FDSHAP.DAT", "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        g_resources.fdshap.size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        g_resources.fdshap.data = (byte*)malloc(g_resources.fdshap.size);
        fread(g_resources.fdshap.data, 1, g_resources.fdshap.size, fp);
        fclose(fp);
        parse_dat_handle(&g_resources.fdshap);
        printf("[资源] FDSHAP.DAT: %d个资源\n", g_resources.fdshap.resource_count);
    }
    
    // 加载FDTXT.DAT文本资源
    printf("[资源] 加载FDTXT.DAT...\n");
    fp = fopen("FDTXT.DAT", "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        g_resources.fdtxt.size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        g_resources.fdtxt.data = (byte*)malloc(g_resources.fdtxt.size);
        fread(g_resources.fdtxt.data, 1, g_resources.fdtxt.size, fp);
        fclose(fp);
        parse_dat_handle(&g_resources.fdtxt);
        printf("[资源] FDTXT.DAT: %d个资源\n", g_resources.fdtxt.resource_count);
    }
    
    // 加载FDMUS.DAT音乐
    printf("[资源] 加载FDMUS.DAT...\n");
    fp = fopen("FDMUS.DAT", "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long mus_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        byte* mus_data = (byte*)malloc(mus_size);
        fread(mus_data, 1, mus_size, fp);
        fclose(fp);
        printf("[资源] FDMUS.DAT已加载 (%d字节, XMI格式)\n", (int)mus_size);
        free(mus_data);
    }
    
    return true;
}

// 解析DAT文件的资源表 (支持4字节和8字节两种格式)
static bool parse_dat_handle(DatHandle* dh) {
    if (!dh->data || dh->size < 6) return false;
    if (memcmp(dh->data, "LLLLLL", 6) != 0) return false;
    
    // 尝试4字节格式: 每个条目4字节,计算下一个偏移作为结束位置
    int count4 = 0;
    int offset4 = 6;
    while (offset4 + 4 <= (int)dh->size) {
        dword s = *(dword*)(dh->data + offset4);
        if (s >= dh->size) break;
        count4++;
        offset4 += 4;
    }
    
    // 尝试8字节格式: 每个条目8字节 [offset][end_offset]
    int count8 = 0;
    int offset8 = 6;
    while (offset8 + 8 <= (int)dh->size) {
        dword s = *(dword*)(dh->data + offset8);
        dword e = *(dword*)(dh->data + offset8 + 4);
        if (s >= dh->size) break;
        // 结束位置必须大于起始位置
        if (e <= s) { offset8 += 8; continue; }
        count8++;
        offset8 += 8;
    }
    
    // 优先选择8字节格式,因为原始代码使用 fseek(fp, 4*index+6) 读8字节
    int count = count8;
    int entry_size = 8;
    
    // 如果8字节格式条目数太少或4字节格式条目数更合理，使用4字节格式
    // FDOTHER.DAT: 4字节格式有103条目，比8字节格式更合理
    if (count4 > count8) {
        count = count4;
        entry_size = 4;
    }
    
    printf("[调试] DAT解析: 8字节格式=%d个资源, 4字节格式=%d个资源, 选择=%d(每条%d字节)\n", 
           count8, count4, count, entry_size);
    
    // 动态分配
    dh->max_resources = count;
    dh->starts = (dword*)malloc(count * sizeof(dword));
    dh->ends = (dword*)malloc(count * sizeof(dword));
    if (!dh->starts || !dh->ends) return false;
    
    // 填充数据 - 重新计算entry_size
    int fill_entry_size = entry_size;
    
    dh->resource_count = 0;
    int fill_pos = 6;
    
    if (fill_entry_size == 8) {
        // 8字节格式: [offset][size]
        while (fill_pos + 8 <= (int)dh->size && dh->resource_count < count) {
            dword s = *(dword*)(dh->data + fill_pos);
            dword e = *(dword*)(dh->data + fill_pos + 4);
            if (s >= dh->size) break;
            if (e <= s) { fill_pos += 8; continue; }
            dh->starts[dh->resource_count] = s;
            dh->ends[dh->resource_count] = e;
            dh->resource_count++;
            fill_pos += 8;
        }
    } else {
        // 4字节格式: 只有offset,需要计算下一个offset作为end
        while (fill_pos + 4 <= (int)dh->size && dh->resource_count < count) {
            dword s = *(dword*)(dh->data + fill_pos);
            dh->starts[dh->resource_count] = s;
            
            // 查找下一个偏移
            int next_pos = fill_pos + 4;
            dword next_s = 0;
            if (next_pos + 4 <= (int)dh->size) {
                next_s = *(dword*)(dh->data + next_pos);
            } else {
                next_s = dh->size;
            }
            dh->ends[dh->resource_count] = next_s;
            
            if (s >= dh->size) break;
            dh->resource_count++;
            fill_pos += 4;
        }
    }
    
    return true;
}

// 释放DatHandle资源
static void free_dat_handle(DatHandle* dh) {
    if (dh->starts) free(dh->starts);
    if (dh->ends) free(dh->ends);
    if (dh->data) free(dh->data);
    memset(dh, 0, sizeof(DatHandle));
}

// 获取DAT资源数据
static byte* get_dat_resource(DatHandle* dh, int index, dword* out_size) {
    if (index < 0 || index >= dh->resource_count) return NULL;
    if (out_size) *out_size = dh->ends[index] - dh->starts[index];
    return dh->data + dh->starts[index];
}

// RLE解压函数 - 基于IDA Pro sub_4E98D分析
static int decompress_rle_simple(byte* src, int src_size, byte* dst, int dst_stride) {
    if (src_size < 4) {
        memcpy(dst, src, src_size);
        return src_size;
    }
    
    unsigned short count = *(unsigned short*)src;      // 行字节数
    unsigned short num_rows = *(unsigned short*)(src + 2); // 迭代次数
    
    src += 4;
    int src_left = src_size - 4;
    int row_gap = dst_stride - count;
    
    // 外层循环: 执行num_rows次(不是dst_height!)
    for (int row_iter = num_rows; row_iter > 0; row_iter--) {
        unsigned short remaining = count;
        
        while (remaining > 0 && src_left > 0) {
            unsigned char val = *src++;
            src_left--;
            
            unsigned int num = (val & 0x3F) + 1;
            if (num > remaining) num = remaining;
            
            if ((val & 0x80) == 0) {
                // 00xxxxxx: 复制num字节从src
                if (src_left >= (int)num) {
                    memcpy(dst, src, num);
                    src += num;
                    src_left -= num;
                    dst += num;
                }
            } else if ((val & 0x40) == 0) {
                // 01xxxxxx: 填充num字节(0)
                memset(dst, 0, num);
                dst += num;
            } else if ((val & 0x20) == 0) {
                // 10xxxxxx: 复制num字节(单字节循环)
                for (unsigned int i = 0; i < num; i++) {
                    *dst++ = *src++;
                    src_left--;
                }
            } else {
                // 11xxxxxx: 填充num字节, remaining递减2倍!
                byte fill = *src++;
                src_left--;
                remaining -= num * 2;
                for (unsigned int i = 0; i < num; i++) {
                    *dst++ = fill;
                }
                continue; // 不要执行后面的remaining -= num
            }
            remaining -= num;
        }
        
        dst += row_gap;
    }
    
    return 0;
}

// 加载指定关卡的数据
static bool load_level(int level) {
    int lvl0 = level - 1;
    if (lvl0 < 0) lvl0 = 0;
    
    // 释放旧数据
    if (g_resources.field_data) { free(g_resources.field_data); g_resources.field_data = NULL; }
    if (g_resources.level_header) { free(g_resources.level_header); g_resources.level_header = NULL; }
    if (g_resources.level_shapes) { free(g_resources.level_shapes); g_resources.level_shapes = NULL; }
    
    // 加载FDFIELD资源0 - RLE压缩的场地数据
    dword fld_size;
    byte* fld_res = get_dat_resource(&g_resources.fdfield, lvl0 * 3, &fld_size);
    if (fld_res && fld_size >= 4) {
        g_resources.field_width = *(word*)fld_res;
        g_resources.field_height = *(word*)(fld_res + 2);
        
        // 分配解压缓冲区 (最大64000字节 = 320x200)
        g_resources.field_data = (byte*)calloc(64000, 1);
        g_resources.field_stride = g_resources.field_width;
        
        // 解压场地数据
        printf("[关卡] 场地尺寸: %dx%d\n", g_resources.field_width, g_resources.field_height);
        decompress_rle_simple(fld_res + 4, fld_size - 4, g_resources.field_data, 
                       g_resources.field_width);
    }
    
    // 加载FDFIELD资源1 - 关卡头信息
    dword hdr_size;
    g_resources.level_header = get_dat_resource(&g_resources.fdfield, lvl0 * 3 + 1, &hdr_size);
    if (g_resources.level_header && hdr_size >= 4) {
        int max_pieces = *(byte*)(g_resources.level_header + 1);
        g_resources.piece_count = max_pieces;
        printf("[关卡] 最大拼图块: %d\n", max_pieces);
    }
    
    // 加载FDSHAP形状数据
    // 索引 = 2 * max_level
    int shap_idx = lvl0 * 2;
    dword shap_size;
    g_resources.level_shapes = get_dat_resource(&g_resources.fdshap, shap_idx, &shap_size);
    if (g_resources.level_shapes) {
        printf("[关卡] 形状数据: %d字节\n", shap_size);
    }
    
    return true;
}

// 加载指定关卡的数据
static Sprite get_sprite_22(int index) {
    Sprite s = {22, 22, NULL, 484};
    if (index >= 0 && index < g_resources.sprite_count_22 && g_resources.sprites_22) {
        s.data = g_resources.sprites_22 + index * 484;
    }
    return s;
}



// 设置调色板到渲染器
static void apply_palette() {
    if (!g_resources.palette) return;
    
    // 6位转8位
    byte palette8[768];
    for (int i = 0; i < 256; i++) {
        byte r6 = g_resources.palette[i * 3] & 0x3F;
        byte g6 = g_resources.palette[i * 3 + 1] & 0x3F;
        byte b6 = g_resources.palette[i * 3 + 2] & 0x3F;
        palette8[i * 3] = (r6 << 2) | (r6 >> 4);
        palette8[i * 3 + 1] = (g6 << 2) | (g6 >> 4);
        palette8[i * 3 + 2] = (b6 << 2) | (b6 >> 4);
        g_render.palette[i][0] = palette8[i * 3];
        g_render.palette[i][1] = palette8[i * 3 + 1];
        g_render.palette[i][2] = palette8[i * 3 + 2];
    }
    
#ifdef USE_SDL
    if (g_sdl_active) {
        fd2_sdl_set_palette_6bit(0, 255, g_resources.palette);
    }
#endif
}

// 清屏
static void clear_screen(byte color) {
    memset(g_render.screen_buffer, color, SCREEN_WIDTH * SCREEN_HEIGHT);
}

// 绘制精灵到屏幕
static void draw_sprite(int x, int y, Sprite sprite, byte transparent_color) {
    if (!sprite.data) return;
    for (int dy = 0; dy < sprite.height; dy++) {
        for (int dx = 0; dx < sprite.width; dx++) {
            int sx = x + dx;
            int sy = y + dy;
            if (sx >= 0 && sx < SCREEN_WIDTH && sy >= 0 && sy < SCREEN_HEIGHT) {
                byte color = sprite.data[dy * sprite.width + dx];
                if (color != transparent_color) {
                    g_render.screen_buffer[sy * SCREEN_WIDTH + sx] = color;
                }
            }
        }
    }
}



// 绘制矩形
static void draw_rect(int x, int y, int w, int h, byte color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            int px = x + dx;
            int py = y + dy;
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                g_render.screen_buffer[py * SCREEN_WIDTH + px] = color;
            }
        }
    }
}

// 绘制文字（简单的位图字体）
static void draw_text(int x, int y, const char* text, byte color) {
    (void)color;
    int cx = x;
    while (*text) {
        unsigned char ch = (unsigned char)*text;
        if (ch >= 32 && ch < 127) {
            int char_idx = ch - 32;
            Sprite font = get_sprite_22(char_idx);
            if (font.data) {
                draw_sprite(cx, y, font, 0);
            }
            cx += font.width + 1;
        }
        text++;
    }
}



// 绘制选关屏幕
static void draw_level_select() {
    clear_screen(0);
    
    // 绘制标题
    draw_text(100, 20, "SELECT LEVEL", 255);
    
    // 绘制关卡网格
    int grid_x = 60;
    int grid_y = 60;
    int cell_size = 30;
    int cols = 10;
    int rows = 5;
    
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int level = row * cols + col + 1;
            int cx = grid_x + col * (cell_size + 4);
            int cy = grid_y + row * (cell_size + 4);
            
            // 绘制单元格背景
            byte bg = (level == g_machine.current_level) ? 200 : 50;
            draw_rect(cx, cy, cell_size, cell_size, bg);
            
            // 绘制边框
            for (int i = 0; i < cell_size; i++) {
                if (cx + i < SCREEN_WIDTH && cy < SCREEN_HEIGHT)
                    g_render.screen_buffer[cy * SCREEN_WIDTH + cx + i] = 255;
                if (cx + i < SCREEN_WIDTH && cy + cell_size - 1 < SCREEN_HEIGHT)
                    g_render.screen_buffer[(cy + cell_size - 1) * SCREEN_WIDTH + cx + i] = 255;
            }
            for (int i = 0; i < cell_size; i++) {
                if (cy + i < SCREEN_HEIGHT && cx < SCREEN_WIDTH)
                    g_render.screen_buffer[(cy + i) * SCREEN_WIDTH + cx] = 255;
                if (cy + i < SCREEN_HEIGHT && cx + cell_size - 1 < SCREEN_WIDTH)
                    g_render.screen_buffer[(cy + i) * SCREEN_WIDTH + cx + cell_size - 1] = 255;
            }
            
            // 绘制关卡号
            if (level <= 50) {
                char buf[4];
                sprintf(buf, "%d", level);
                int num_x = cx + 8;
                int num_y = cy + 8;
                draw_text(num_x, num_y, buf, 32);
            }
        }
    }
    
    // 绘制操作提示
    draw_text(80, 185, "PRESS 1-9,0 FOR LEVEL  Q:QUIT", 150);
}

// 绘制拼图游戏屏幕
static void draw_game_screen() {
    clear_screen(0);
    
    // 绘制顶部状态栏
    draw_rect(0, 0, SCREEN_WIDTH, 24, 100);
    
    char buf[64];
    sprintf(buf, "LV:%d", g_machine.current_level);
    draw_text(4, 4, buf, 255);
    sprintf(buf, "SC:%d", g_machine.score);
    draw_text(60, 4, buf, 255);
    sprintf(buf, "MV:%d", g_machine.moves);
    draw_text(120, 4, buf, 255);
    
    // 如果有解压的场地数据，先绘制它
    if (g_resources.field_data && g_resources.field_width > 0) {
        int dst_x = 0;
        int dst_y = 24;
        
        // 缩放场地数据到屏幕 (2x scale to fit 320 width)
        int scale = 1;
        if (g_resources.field_width <= 320) scale = 1;
        
        int draw_w = g_resources.field_width * scale;
        int draw_h = g_resources.field_height * scale;
        
        // 确保不超过屏幕
        if (dst_x + draw_w > SCREEN_WIDTH) draw_w = SCREEN_WIDTH - dst_x;
        if (dst_y + draw_h > SCREEN_HEIGHT - 20) draw_h = SCREEN_HEIGHT - 20 - dst_y;
        
        for (int y = 0; y < draw_h; y++) {
            int src_y = y / scale;
            for (int x = 0; x < draw_w; x++) {
                int src_x = x / scale;
                if (src_x < g_resources.field_width && src_y < g_resources.field_height) {
                    int idx = src_y * g_resources.field_width + src_x;
                    byte color = g_resources.field_data[idx];
                    int px = dst_x + x;
                    int py = dst_y + y;
                    if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                        g_render.screen_buffer[py * SCREEN_WIDTH + px] = color;
                    }
                }
            }
        }
    } else {
        // 没有场地数据时绘制默认背景
        // 使用sprite绘制背景演示
        int bg_y = 24;
        for (int row = 0; row < 8 && bg_y < SCREEN_HEIGHT - 24; row++) {
            for (int col = 0; col < 14 && col * 24 + 40 < SCREEN_WIDTH; col++) {
                int sprite_idx = (row * 14 + col) % g_resources.sprite_count_22;
                Sprite s = get_sprite_22(sprite_idx);
                draw_sprite(40 + col * 24, bg_y, s, 0);
            }
            bg_y += 22;
        }
    }
    
    // 绘制拼图块提示区
    int hint_x = SCREEN_WIDTH - 100;
    int hint_y = 28;
    draw_text(hint_x, hint_y, "SHAPES:", 200);
    hint_y += 22;
    
    for (int i = 0; i < 4; i++) {
        int sprite_idx = (g_machine.frame_count / 20 + i) % g_resources.sprite_count_22;
        Sprite s = get_sprite_22(sprite_idx);
        draw_sprite(hint_x + i * 24, hint_y, s, 0);
    }
    
    // 高亮当前选中的拼图块
    if (g_machine.highlight_x >= 0 && g_machine.highlight_y >= 0) {
        int hx = 40 + g_machine.highlight_x * 24;
        int hy = 24 + g_machine.highlight_y * 22;
        // 画边框高亮
        for (int i = 0; i < 24; i++) {
            if (hy >= 0 && hy < SCREEN_HEIGHT && hx + i >= 0 && hx + i < SCREEN_WIDTH)
                g_render.screen_buffer[hy * SCREEN_WIDTH + hx + i] = 255;
            if (hy + 22 >= 0 && hy + 22 < SCREEN_HEIGHT && hx + i >= 0 && hx + i < SCREEN_WIDTH)
                g_render.screen_buffer[(hy + 22) * SCREEN_WIDTH + hx + i] = 255;
        }
        for (int i = 0; i < 22; i++) {
            if (hy + i >= 0 && hy + i < SCREEN_HEIGHT && hx >= 0 && hx < SCREEN_WIDTH)
                g_render.screen_buffer[(hy + i) * SCREEN_WIDTH + hx] = 255;
            if (hy + i >= 0 && hy + i < SCREEN_HEIGHT && hx + 23 >= 0 && hx + 23 < SCREEN_WIDTH)
                g_render.screen_buffer[(hy + i) * SCREEN_WIDTH + hx + 23] = 255;
        }
    }
    
    // 底部操作提示
    draw_rect(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 20, 80);
    draw_text(4, SCREEN_HEIGHT - 15, "ARROWS:MOVE SPACE:SELECT R:ROTATE P:PAUSE Q:QUIT", 255);
}



// 绘制胜利画面
static void draw_win_screen() {
    clear_screen(0);
    
    draw_text(100, 50, "CONGRATULATIONS!", 220);
    
    char buf[64];
    sprintf(buf, "LEVEL %d COMPLETE!", g_machine.current_level);
    draw_text(80, 80, buf, 200);
    
    sprintf(buf, "SCORE: %d", g_machine.score);
    draw_text(120, 110, buf, 180);
    
    sprintf(buf, "MOVES: %d", g_machine.moves);
    draw_text(120, 130, buf, 180);
    
    if (g_machine.frame_count % 40 < 20) {
        draw_text(100, 160, "PRESS ENTER FOR NEXT LEVEL", 255);
    }
}

// 状态机更新
static void update_state() {
    g_machine.frame_count++;
}

// 渲染当前状态
static void render_state() {
    switch (g_machine.current_state) {
        case STATE_INIT:
            clear_screen(0);
            draw_text(100, 90, "LOADING...", 200);
            break;
        case STATE_TITLE:
            g_ani_res_idx = 0;
            if (g_machine.frame_count % 3 == 0) {
                g_ani_frame++;
                update_ani_frame();
            }
            memcpy(g_render.screen_buffer, ani_screen_buf, sizeof(ani_screen_buf));
            break;
        case STATE_LEVEL_SELECT:
            draw_level_select();
            break;
        case STATE_PLAYING:
            draw_game_screen();
            break;
        case STATE_WIN:
            draw_win_screen();
            break;
        case STATE_PAUSED:
            draw_game_screen();
            draw_text(130, 90, "PAUSED", 255);
            draw_text(100, 110, "PRESS P TO RESUME", 200);
            break;
        default:
            clear_screen(0);
            break;
    }
}

// 显示画面
static void present() {
#ifdef USE_SDL
    if (g_sdl_active) {
        fd2_sdl_render_frame(g_render.screen_buffer);
    } else
#endif
    {
        static const char* chars = " .:-=+*#%@";
        for (int y = 0; y < SCREEN_HEIGHT; y += 2) {
            for (int x = 0; x < SCREEN_WIDTH; x += 2) {
                byte color = g_render.screen_buffer[y * SCREEN_WIDTH + x];
                int idx = (color * 9) / 256;
                putchar(chars[idx]);
            }
            putchar('\n');
        }
    }
}

// 控制台键盘输入（非SDL模式）
#ifdef _WIN32
#include <conio.h>
static int console_get_key() {
    if (_kbhit()) {
        return _getch();
    }
    return -1;
}
#else
#include <termios.h>
#include <unistd.h>
static int console_get_key() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

static void handle_input_key(int key) {
    switch (g_machine.current_state) {
        case STATE_TITLE:
            if (key == 13) { // Enter
                g_machine.next_state = STATE_LEVEL_SELECT;
            }
            break;
            
        case STATE_LEVEL_SELECT:
            if (key >= '1' && key <= '9') {
                g_machine.current_level = key - '1' + 1;
            } else if (key == '0') {
                g_machine.current_level = 10;
            } else if (key >= 'a' && key <= 'z') {
                // Arrow keys: W=up, S=down, A=left, D=right
                // In SDL, arrow keys are mapped to WASD
            } else if (key == 'Q' || key == 'q' || key == 27) {
                g_machine.running = false;
            } else if (key == 13) { // Enter
                g_machine.next_state = STATE_PLAYING;
            }
            break;
            
        case STATE_PLAYING:
            if (key == 'P' || key == 'p') {
                g_machine.next_state = STATE_PAUSED;
            } else if (key == 'Q' || key == 'q' || key == 27) {
                g_machine.next_state = STATE_TITLE;
            } else if (key == 'w' || key == 'W' || key == 72) { // Up (W or Arrow Up)
                g_machine.highlight_y--;
                if (g_machine.highlight_y < 0) g_machine.highlight_y = 7;
            } else if (key == 's' || key == 'S' || key == 80) { // Down
                g_machine.highlight_y++;
                if (g_machine.highlight_y >= 8) g_machine.highlight_y = 0;
            } else if (key == 'a' || key == 'A' || key == 75) { // Left
                g_machine.highlight_x--;
                if (g_machine.highlight_x < 0) g_machine.highlight_x = 12;
            } else if (key == 'd' || key == 'D' || key == 77) { // Right
                g_machine.highlight_x++;
                if (g_machine.highlight_x >= 13) g_machine.highlight_x = 0;
            } else if (key == ' ') { // Space: place/move piece
                g_machine.moves++;
                // 得分逻辑
                if (g_machine.moves > 100) {
                    g_machine.score = 1000 - (g_machine.moves - 100) * 5;
                } else {
                    g_machine.score = 1000 + (100 - g_machine.moves) * 10;
                }
                if (g_machine.score < 0) g_machine.score = 0;
                
                // 模拟过关 (当移动足够多次后可能触发)
                static int win_trigger = 0;
                win_trigger++;
                if (win_trigger > 50) {
                    g_machine.next_state = STATE_WIN;
                    win_trigger = 0;
                }
            } else if (key == 'r' || key == 'R') { // R: Rotate piece
                g_machine.selected_piece = (g_machine.selected_piece + 1) % g_resources.sprite_count_22;
            }
            break;
            
        case STATE_WIN:
            if (key == 13) { // Enter
                g_machine.current_level++;
                if (g_machine.current_level > g_resources.fdfield.resource_count / 3) 
                    g_machine.current_level = 1;
                g_machine.next_state = STATE_PLAYING;
            } else if (key == 'Q' || key == 'q' || key == 27) {
                g_machine.next_state = STATE_TITLE;
            }
            break;
            
        case STATE_PAUSED:
            if (key == 'P' || key == 'p') {
                g_machine.next_state = STATE_PLAYING;
            } else if (key == 'Q' || key == 'q' || key == 27) {
                g_machine.next_state = STATE_TITLE;
            }
            break;
    }
}

// 处理SDL事件
static bool handle_sdl_events() {
#ifdef USE_SDL
    if (!g_sdl_active) return true;
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            g_machine.running = false;
            return false;
        }
        if (event.type == SDL_KEYDOWN) {
            int key = event.key.keysym.sym;
            
            // Map special keys
            if (key == SDLK_ESCAPE) key = 27;
            else if (key == SDLK_RETURN) key = 13;
            else if (key == SDLK_SPACE) key = ' ';
            else if (key == SDLK_BACKSPACE) key = 8;
            else if (key == SDLK_TAB) key = 9;
            else if (key >= SDLK_0 && key <= SDLK_9) key = '0' + (key - SDLK_0);
            else if (key >= SDLK_a && key <= SDLK_z) key = 'a' + (key - SDLK_a);
            else if (key >= SDLK_KP_0 && key <= SDLK_KP_9) key = '0' + (key - SDLK_KP_0);
            else if (key == SDLK_KP_PLUS) key = '+';
            else if (key == SDLK_KP_MINUS) key = '-';
            // Arrow keys
            else if (key == SDLK_UP) key = 72;    // Key code for up arrow
            else if (key == SDLK_DOWN) key = 80;  // Key code for down arrow
            else if (key == SDLK_LEFT) key = 75; // Key code for left arrow
            else if (key == SDLK_RIGHT) key = 77; // Key code for right arrow
            else if (key == SDLK_r) key = 'r';
            else if (key == SDLK_p) key = 'p';
            else if (key == SDLK_q || key == SDLK_ESCAPE) key = 'q';
            else key = -1; // Ignore other keys
            
            if (key != -1) {
                handle_input_key(key);
            }
        }
    }
#endif
    return true;
}

// 延迟
static void delay_ms(uint32_t ms) {
#ifdef USE_SDL
    if (g_sdl_active) {
        SDL_Delay(ms);
    } else
#endif
    {
        clock_t start = clock();
        while ((unsigned long)((clock() - start) * 1000 / CLOCKS_PER_SEC) < ms) {}
    }
}

// 主循环
static int game_loop() {
    printf("[游戏] 开始游戏主循环...\n");
    
    while (g_machine.running) {
        // 状态转换
        if (g_machine.next_state != g_machine.current_state) {
            printf("[状态] %d -> %d\n", g_machine.current_state, g_machine.next_state);
            g_machine.current_state = g_machine.next_state;
            
            // 进入游戏状态时加载关卡
            if (g_machine.current_state == STATE_PLAYING) {
                printf("[关卡] 加载关卡 %d...\n", g_machine.current_level);
                load_level(g_machine.current_level);
                g_machine.moves = 0;
                g_machine.score = 0;
            }
        }
        
        // 处理SDL事件
        if (!handle_sdl_events()) break;
        
#ifndef USE_SDL
        // 处理控制台键盘输入
        int ch = console_get_key();
        if (ch != -1) {
            if (ch == 13) handle_input_key(13);
            else if (ch >= '0' && ch <= '9') handle_input_key(ch);
            else if (ch == ' ' || ch == ' ') handle_input_key(' ');
            else if (ch == 'p' || ch == 'P') handle_input_key('P');
            else if (ch == 'q' || ch == 'Q') handle_input_key('Q');
            else if (ch == 'w' || ch == 'W') handle_input_key('W');
            else if (ch == 's' || ch == 'S') handle_input_key('S');
            else if (ch == 'a' || ch == 'A') handle_input_key('A');
            else if (ch == 'd' || ch == 'D') handle_input_key('D');
        }
#endif
        
        // 更新
        update_state();
        
        // 渲染
        render_state();
        present();
        
        // 帧率控制
        delay_ms(FRAME_DELAY);
    }
    
    return 0;
}

// 设置控制台输出编码为UTF-8
static void init_console_utf8(void) {
#ifdef _WIN32
    // 设置控制台代码页为UTF-8 (65001)
    SetConsoleOutputCP(650001);
    // 设置控制台代码页为UTF-8
    SetConsoleCP(650001);
    
    // 尝试启用VT100转义序列支持 (Windows 10+)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

// 主函数
int main(int argc, char* argv[]) {
    // 初始化控制台UTF-8编码
    init_console_utf8();
    
    bool skip_startup = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--skip") == 0) {
            skip_startup = true;
        }
    }
    
    printf("============================================\n");
    printf("  FD2.exe 重新实现 - Puzzle Beauty\n");
    printf("  基于IDA Pro MCP服务器逆向工程\n");
    printf("============================================\n\n");
    
    // 初始化游戏状态机
    init_game_machine(&g_machine);
    memset(&g_render, 0, sizeof(RenderState));
    memset(&g_resources, 0, sizeof(ResourceCache));
    
#ifdef USE_SDL
    if (fd2_sdl_init_renderer()) {
        g_sdl_active = true;
        printf("[SDL] SDL2渲染系统已启用\n");
        printf("[SDL] 窗口尺寸: %dx%d (逻辑分辨率: 320x200, 缩放: 2x)\n", SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2);
    } else {
        printf("[SDL] SDL2初始化失败，使用控制台模式\n");
    }
#endif
    
    // 加载资源
    printf("\n[游戏] 加载游戏资源...\n");
    if (!load_resources()) {
        printf("[错误] 资源加载失败\n");
        return 1;
    }
    
    // 应用调色板
    apply_palette();
    
    printf("\n[游戏] 资源加载完成\n");
    printf("[游戏] 精灵: %d个(22x22), %d个(32x32)\n", 
           g_resources.sprite_count_22, g_resources.sprite_count_32);
    
    // 运行完整的启动动画序列
    if (skip_startup) {
        printf("\n[游戏] 跳过启动动画\n");
        g_startup.complete = true;
    } else {
        printf("\n[游戏] 运行完整启动序列...\n");
        run_full_startup_sequence();
    }
    
    // 加载ANI动画数据用于游戏内动画
    printf("[游戏] 加载ANI动画...\n");
    g_ani_res_idx = 0;
    g_ani_frame = 0;
    int r = load_ani_cached(0);
    printf("[游戏] load_ani_cached returned %d, blocks=%d\n", r, g_ani_block_count);
    printf("[游戏] ANI动画: %d帧\n", g_ani_block_count);
    
    // 预加载第一帧
    memset(ani_screen_buf, 0, 64000);
    update_ani_frame();
    
    // 运行游戏
    game_loop();
    
    // 清理
    printf("\n[游戏] 清理资源...\n");
    if (g_resources.palette) free(g_resources.palette);
    if (g_resources.sprites_22) free(g_resources.sprites_22);
    if (g_resources.sprites_32) free(g_resources.sprites_32);
    free_dat_handle(&g_resources.fdother);
    free_dat_handle(&g_resources.fdfield);
    free_dat_handle(&g_resources.fdshap);
    free_dat_handle(&g_resources.fdtxt);
    free_dat_handle(&g_startup.figani);
    if (g_resources.field_data) free(g_resources.field_data);
    
#ifdef USE_SDL
    if (g_sdl_active) {
        fd2_sdl_cleanup();
    }
#endif
    
    printf("[游戏] 退出游戏\n");
    return 0;
}
