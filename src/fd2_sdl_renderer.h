/**
 * FD2 SDL渲染系统头文件
 * 基于IDA Pro MCP服务器对fd2.exe图形系统的分析
 */

#ifndef FD2_SDL_RENDERER_H
#define FD2_SDL_RENDERER_H

#ifdef USE_SDL
#include <SDL2/SDL.h>
#endif
#include <stdbool.h>

// 游戏常量
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_SIZE 256
#define SCALE_FACTOR 2

typedef unsigned char byte;

#ifdef USE_SDL
// SDL渲染器结构
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    Uint32* texture_buffer;  // 32位颜色缓冲区
    byte palette[PALETTE_SIZE][3];  // RGB调色板（8位）
    byte screen_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];  // 8位索引缓冲区
    bool initialized;
} SDLRenderer;
#endif

#ifdef USE_SDL
// 函数声明
bool fd2_sdl_init_renderer();
void fd2_sdl_set_palette(int start, int end, const byte* palette_data);
void fd2_sdl_set_palette_6bit(int start, int end, const byte* palette_data);
void fd2_sdl_clear_screen(byte color_index);
void fd2_sdl_plot_pixel(int x, int y, byte color_index);
void fd2_sdl_draw_rect(int x, int y, int width, int height, byte color_index);
void fd2_sdl_draw_image(int x, int y, int width, int height, const byte* image_data);
void fd2_sdl_update_texture();
void fd2_sdl_render_frame(const byte* screen_buffer);
void fd2_sdl_render_ani_frame(const byte* ani_buffer, int width, int height);
bool fd2_sdl_process_events();
void fd2_sdl_cleanup();

// 获取全局渲染器实例
SDLRenderer* get_sdl_renderer();
#endif

#endif // FD2_SDL_RENDERER_H