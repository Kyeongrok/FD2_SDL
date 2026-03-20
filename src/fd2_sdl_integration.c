/**
 * FD2 SDL集成示例
 * 展示如何使用SDL渲染FD2游戏资源
 * 基于IDA Pro MCP服务器对渲染管线的分析
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 假设SDL已经包含
// #include <SDL2/SDL.h>

// 模拟SDL类型（用于编译）
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef unsigned int Uint32;
typedef unsigned char byte;

// 游戏常量
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_SIZE 256

// FD2渲染上下文
typedef struct {
    // SDL对象
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    Uint32* texture_buffer;
    
    // 游戏状态
    byte palette[PALETTE_SIZE][3];  // 8位RGB调色板
    byte screen_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    
    // 资源数据
    byte* fdother_data;
    byte* ani_data;
    int fdother_size;
    int ani_size;
} FD2RenderContext;

// 基于IDA分析的函数
void fd2_set_palette_6bit(FD2RenderContext* ctx, int start, int end, const byte* palette_6bit) {
    // 根据sub_11D40函数分析
    // 设置调色板，将6位颜色转换为8位
    for (int i = start; i <= end && i < PALETTE_SIZE; i++) {
        byte r6 = palette_6bit[i * 3] & 0x3F;
        byte g6 = palette_6bit[i * 3 + 1] & 0x3F;
        byte b6 = palette_6bit[i * 3 + 2] & 0x3F;
        
        // 6位扩展到8位（重复高2位）
        ctx->palette[i][0] = (r6 << 2) | (r6 >> 4);  // R
        ctx->palette[i][1] = (g6 << 2) | (g6 >> 4);  // G
        ctx->palette[i][2] = (b6 << 2) | (b6 >> 4);  // B
    }
}

void fd2_clear_screen(FD2RenderContext* ctx, byte color_index) {
    // 根据图形系统分析
    memset(ctx->screen_buffer, color_index, SCREEN_WIDTH * SCREEN_HEIGHT);
}

void fd2_plot_pixel(FD2RenderContext* ctx, int x, int y, byte color_index) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        ctx->screen_buffer[y * SCREEN_WIDTH + x] = color_index;
    }
}

void fd2_draw_image(FD2RenderContext* ctx, int x, int y, int width, int height, const byte* image_data) {
    // 根据渲染管线分析
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int src_idx = dy * width + dx;
            if (src_idx < width * height) {
                fd2_plot_pixel(ctx, x + dx, y + dy, image_data[src_idx]);
            }
        }
    }
}

// 资源解码函数
byte* decode_fdother_resource(FD2RenderContext* ctx, int resource_index, int* width, int* height) {
    // 根据FDOTHER.DAT格式分析
    if (!ctx->fdother_data) return NULL;
    
    // 解析资源表（从偏移6开始）
    int offset = 6 + resource_index * 8;
    if (offset + 8 > ctx->fdother_size) return NULL;
    
    // 读取资源位置
    unsigned int start_offset = *(unsigned int*)(ctx->fdother_data + offset);
    unsigned int end_offset = *(unsigned int*)(ctx->fdother_data + offset + 4);
    
    if (start_offset >= end_offset || end_offset > ctx->fdother_size) {
        return NULL;
    }
    
    int size = end_offset - start_offset;
    
    // 尝试常见图像尺寸
    int common_widths[] = {320, 398, 32, 256, 128, 64};
    int common_heights[] = {200, 25, 321, 256, 128, 64};
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            if (common_widths[i] * common_heights[j] == size) {
                *width = common_widths[i];
                *height = common_heights[j];
                return ctx->fdother_data + start_offset;
            }
        }
    }
    
    return NULL;
}

byte* decode_ani_resource(FD2RenderContext* ctx, int resource_index, int* width, int* height) {
    // 根据ANI.DAT格式分析
    if (!ctx->ani_data) return NULL;
    
    // 解析资源表（从偏移6开始）
    int offset = 6 + resource_index * 8;
    if (offset + 8 > ctx->ani_size) return NULL;
    
    // 读取资源位置
    unsigned int start_offset = *(unsigned int*)(ctx->ani_data + offset);
    unsigned int end_offset = *(unsigned int*)(ctx->ani_data + offset + 4);
    
    if (start_offset >= end_offset || end_offset > ctx->ani_size) {
        return NULL;
    }
    
    // 读取8字节头
    unsigned int first_dword = *(unsigned int*)(ctx->ani_data + start_offset);
    
    // 使用第一个双字作为实际偏移量
    unsigned int actual_offset = first_dword;
    if (actual_offset >= ctx->ani_size) {
        actual_offset = start_offset;
    }
    
    // 读取资源头
    if (actual_offset + 173 > ctx->ani_size) {
        return NULL;
    }
    
    // 从偏移165读取块数量
    unsigned short block_count = *(unsigned short*)(ctx->ani_data + actual_offset + 165);
    
    // 这里需要实现命令解码系统
    // 简化处理：假设数据是原始图像
    
    // 尝试常见尺寸
    int common_widths[] = {320, 398, 32, 256, 128};
    int common_heights[] = {200, 25, 321, 256, 128};
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int size = common_widths[i] * common_heights[j];
            if (actual_offset + size <= ctx->ani_size) {
                *width = common_widths[i];
                *height = common_heights[j];
                return ctx->ani_data + actual_offset;
            }
        }
    }
    
    return NULL;
}

// 渲染FD2帧
void fd2_render_frame(FD2RenderContext* ctx) {
    // 这里可以实现FD2的完整渲染流程
    // 1. 清空屏幕
    // 2. 从FDOTHER.DAT加载背景
    // 3. 从ANI.DAT加载动画帧
    // 4. 渲染UI元素
    // 5. 显示到屏幕
    
    // 简化示例：绘制测试图案
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            byte color = (x + y) % 256;
            fd2_plot_pixel(ctx, x, y, color);
        }
    }
}

// 主函数示例
int main_example() {
    printf("FD2 SDL集成示例\n");
    
    // 这里是概念代码，实际需要SDL初始化
    FD2RenderContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    
    // 加载资源文件
    FILE* fdother_file = fopen("FDOTHER.DAT", "rb");
    if (fdother_file) {
        fseek(fdother_file, 0, SEEK_END);
        ctx.fdother_size = ftell(fdother_file);
        fseek(fdother_file, 0, SEEK_SET);
        ctx.fdother_data = (byte*)malloc(ctx.fdother_size);
        fread(ctx.fdother_data, 1, ctx.fdother_size, fdother_file);
        fclose(fdother_file);
        printf("FDOTHER.DAT加载成功 (%d字节)\n", ctx.fdother_size);
    }
    
    FILE* ani_file = fopen("ANI.DAT", "rb");
    if (ani_file) {
        fseek(ani_file, 0, SEEK_END);
        ctx.ani_size = ftell(ani_file);
        fseek(ani_file, 0, SEEK_SET);
        ctx.ani_data = (byte*)malloc(ctx.ani_size);
        fread(ctx.ani_data, 1, ctx.ani_size, ani_file);
        fclose(ani_file);
        printf("ANI.DAT加载成功 (%d字节)\n", ctx.ani_size);
    }
    
    // 设置调色板
    if (ctx.fdother_data) {
        // 查找调色板资源（假设资源0是调色板）
        int width, height;
        byte* palette_data = decode_fdother_resource(&ctx, 0, &width, &height);
        if (palette_data && width * height == 768) {
            fd2_set_palette_6bit(&ctx, 0, 255, palette_data);
            printf("调色板设置成功\n");
        }
    }
    
    // 渲染测试帧
    fd2_render_frame(&ctx);
    
    // 清理
    if (ctx.fdother_data) free(ctx.fdother_data);
    if (ctx.ani_data) free(ctx.ani_data);
    
    printf("示例完成\n");
    return 0;
}

// 编译说明：
// 1. 安装SDL2开发库
// 2. 使用以下命令编译：
//    gcc -o fd2_sdl fd2_sdl_integration.c -lSDL2 -std=c99
// 3. 运行: ./fd2_sdl