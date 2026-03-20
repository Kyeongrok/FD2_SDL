/**
 * FD2 SDL渲染系统实现
 * 基于IDA Pro MCP服务器对fd2.exe图形系统的分析
 */

#ifdef USE_SDL
#include "fd2_sdl_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 全局渲染器实例
static SDLRenderer renderer;

/**
 * 获取全局渲染器实例
 */
SDLRenderer* get_sdl_renderer() {
    return &renderer;
}

/**
 * 初始化SDL渲染系统
 */
bool fd2_sdl_init_renderer() {
    printf("初始化SDL渲染系统...\n");
    
    // 初始化SDL视频子系统
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL初始化失败: %s\n", SDL_GetError());
        return false;
    }
    
    // 创建窗口
    renderer.window = SDL_CreateWindow(
        "FD2重新实现 - 基于IDA Pro MCP服务器分析",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * SCALE_FACTOR,
        SCREEN_HEIGHT * SCALE_FACTOR,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    
    if (!renderer.window) {
        printf("窗口创建失败: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }
    
    // 创建渲染器
    renderer.renderer = SDL_CreateRenderer(
        renderer.window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!renderer.renderer) {
        printf("渲染器创建失败: %s\n", SDL_GetError());
        SDL_DestroyWindow(renderer.window);
        SDL_Quit();
        return false;
    }
    
    // 设置渲染器逻辑大小
    SDL_RenderSetLogicalSize(renderer.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // 创建纹理（ARGB8888格式）
    renderer.texture = SDL_CreateTexture(
        renderer.renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );
    
    if (!renderer.texture) {
        printf("纹理创建失败: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer.renderer);
        SDL_DestroyWindow(renderer.window);
        SDL_Quit();
        return false;
    }
    
    // 分配纹理缓冲区
    renderer.texture_buffer = (Uint32*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Uint32));
    if (!renderer.texture_buffer) {
        printf("纹理缓冲区分配失败\n");
        SDL_DestroyTexture(renderer.texture);
        SDL_DestroyRenderer(renderer.renderer);
        SDL_DestroyWindow(renderer.window);
        SDL_Quit();
        return false;
    }
    
    // 清空缓冲区
    memset(renderer.screen_buffer, 0, sizeof(renderer.screen_buffer));
    memset(renderer.texture_buffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Uint32));
    
    // 初始化默认调色板（灰度）
    for (int i = 0; i < PALETTE_SIZE; i++) {
        renderer.palette[i][0] = i;  // R
        renderer.palette[i][1] = i;  // G
        renderer.palette[i][2] = i;  // B
    }
    
    renderer.initialized = true;
    printf("SDL渲染系统初始化成功\n");
    printf("窗口尺寸: %dx%d\n", SCREEN_WIDTH * SCALE_FACTOR, SCREEN_HEIGHT * SCALE_FACTOR);
    
    return true;
}

/**
 * 设置调色板
 */
void fd2_sdl_set_palette(int start, int end, const byte* palette_data) {
    if (!renderer.initialized) return;
    
    for (int i = start; i <= end && i < PALETTE_SIZE; i++) {
        int idx = (i - start) * 3;
        renderer.palette[i][0] = palette_data[idx];     // R (8位)
        renderer.palette[i][1] = palette_data[idx + 1]; // G
        renderer.palette[i][2] = palette_data[idx + 2]; // B
    }
}

/**
 * 设置6位调色板（从FD2格式转换）
 */
void fd2_sdl_set_palette_6bit(int start, int end, const byte* palette_data) {
    if (!renderer.initialized) return;
    
    for (int i = start; i <= end && i < PALETTE_SIZE; i++) {
        int idx = (i - start) * 3;
        byte r6 = palette_data[idx] & 0x3F;
        byte g6 = palette_data[idx + 1] & 0x3F;
        byte b6 = palette_data[idx + 2] & 0x3F;
        
        // 6位扩展到8位（重复高2位）
        renderer.palette[i][0] = (r6 << 2) | (r6 >> 4);
        renderer.palette[i][1] = (g6 << 2) | (g6 >> 4);
        renderer.palette[i][2] = (b6 << 2) | (b6 >> 4);
    }
}

/**
 * 清空屏幕
 */
void fd2_sdl_clear_screen(byte color_index) {
    if (!renderer.initialized) return;
    
    memset(renderer.screen_buffer, color_index, SCREEN_WIDTH * SCREEN_HEIGHT);
}

/**
 * 绘制像素
 */
void fd2_sdl_plot_pixel(int x, int y, byte color_index) {
    if (!renderer.initialized) return;
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    
    renderer.screen_buffer[y * SCREEN_WIDTH + x] = color_index;
}

/**
 * 绘制矩形
 */
void fd2_sdl_draw_rect(int x, int y, int width, int height, byte color_index) {
    if (!renderer.initialized) return;
    
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            fd2_sdl_plot_pixel(x + dx, y + dy, color_index);
        }
    }
}

/**
 * 绘制图像（8位调色板索引）
 */
void fd2_sdl_draw_image(int x, int y, int width, int height, const byte* image_data) {
    if (!renderer.initialized) return;
    
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int src_idx = dy * width + dx;
            if (src_idx < width * height) {
                fd2_sdl_plot_pixel(x + dx, y + dy, image_data[src_idx]);
            }
        }
    }
}

/**
 * 将屏幕缓冲区转换为32位纹理缓冲区
 */
void fd2_sdl_update_texture() {
    if (!renderer.initialized) return;
    
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        byte color_index = renderer.screen_buffer[i];
        byte r = renderer.palette[color_index][0];
        byte g = renderer.palette[color_index][1];
        byte b = renderer.palette[color_index][2];
        
        // 转换为ARGB8888格式
        renderer.texture_buffer[i] = (0xFF << 24) | (r << 16) | (g << 8) | b;
    }
    
    // 更新纹理
    SDL_UpdateTexture(renderer.texture, NULL, renderer.texture_buffer, SCREEN_WIDTH * sizeof(Uint32));
}

/**
 * 渲染ANI动画帧
 */
void fd2_sdl_render_ani_frame(const byte* ani_buffer, int width, int height) {
    if (!renderer.initialized) return;
    if (width > SCREEN_WIDTH) width = SCREEN_WIDTH;
    if (height > SCREEN_HEIGHT) height = SCREEN_HEIGHT;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            renderer.screen_buffer[y * SCREEN_WIDTH + x] = ani_buffer[y * width + x];
        }
    }
    
    // 更新纹理
    fd2_sdl_update_texture();
    
    // 清除屏幕
    SDL_SetRenderDrawColor(renderer.renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer.renderer);
    
    // 渲染纹理
    SDL_RenderCopy(renderer.renderer, renderer.texture, NULL, NULL);
    
    // 显示
    SDL_RenderPresent(renderer.renderer);
}

/**
 * 渲染帧
 */
void fd2_sdl_render_frame(const byte* screen_buffer) {
    if (!renderer.initialized) return;
    if (!screen_buffer) return;
    
    // 复制屏幕缓冲区
    memcpy(renderer.screen_buffer, screen_buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
    
    // 更新纹理
    fd2_sdl_update_texture();
    
    // 清除屏幕
    SDL_SetRenderDrawColor(renderer.renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer.renderer);
    
    // 渲染纹理
    SDL_RenderCopy(renderer.renderer, renderer.texture, NULL, NULL);
    
    // 显示
    SDL_RenderPresent(renderer.renderer);
}

/**
 * 处理输入事件
 */
bool fd2_sdl_process_events() {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return false;  // 退出程序
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        return false;  // ESC退出
                    case SDLK_F11:
                        // 切换全屏
                        SDL_SetWindowFullscreen(renderer.window, 
                            (SDL_GetWindowFlags(renderer.window) & SDL_WINDOW_FULLSCREEN) ? 0 : SDL_WINDOW_FULLSCREEN);
                        break;
                    case SDLK_1:
                        // 1倍缩放
                        SDL_SetWindowSize(renderer.window, SCREEN_WIDTH, SCREEN_HEIGHT);
                        break;
                    case SDLK_2:
                        // 2倍缩放
                        SDL_SetWindowSize(renderer.window, SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2);
                        break;
                    case SDLK_3:
                        // 3倍缩放
                        SDL_SetWindowSize(renderer.window, SCREEN_WIDTH * 3, SCREEN_HEIGHT * 3);
                        break;
                    case SDLK_p:
                        // 截图
                        SDL_Surface* surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
                        SDL_RenderReadPixels(renderer.renderer, NULL, SDL_PIXELFORMAT_ARGB8888, surface->pixels, surface->pitch);
                        SDL_SaveBMP(surface, "screenshot.bmp");
                        SDL_FreeSurface(surface);
                        printf("截图已保存: screenshot.bmp\n");
                        break;
                }
                break;
                
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    // 窗口大小改变
                }
                break;
        }
    }
    
    return true;  // 继续运行
}

/**
 * 清理SDL资源
 */
void fd2_sdl_cleanup() {
    if (!renderer.initialized) return;
    
    printf("清理SDL资源...\n");
    
    if (renderer.texture_buffer) {
        free(renderer.texture_buffer);
        renderer.texture_buffer = NULL;
    }
    
    if (renderer.texture) {
        SDL_DestroyTexture(renderer.texture);
        renderer.texture = NULL;
    }
    
    if (renderer.renderer) {
        SDL_DestroyRenderer(renderer.renderer);
        renderer.renderer = NULL;
    }
    
    if (renderer.window) {
        SDL_DestroyWindow(renderer.window);
        renderer.window = NULL;
    }
    
    SDL_Quit();
    renderer.initialized = false;
    
    printf("SDL资源清理完成\n");
}
#endif