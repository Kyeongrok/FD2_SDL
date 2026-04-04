/**
 * FD2 图像解码模块实现
 * 基于 fd2resviewer Python代码移植
 */

#include "../include/fd2_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void palette_init_6bit(Palette* palette, const byte* data) {
    if (!palette || !data) return;
    
    for (int i = 0; i < PALETTE_SIZE; i++) {
        byte r6 = data[i * 3] & 0x3F;
        byte g6 = data[i * 3 + 1] & 0x3F;
        byte b6 = data[i * 3 + 2] & 0x3F;
        
        palette->colors[i][0] = (r6 << 2) | (r6 >> 4);
        palette->colors[i][1] = (g6 << 2) | (g6 >> 4);
        palette->colors[i][2] = (b6 << 2) | (b6 >> 4);
    }
}

int palette_load_from_dat(Palette* palette, const char* dat_file) {
    FILE* fp = fopen(dat_file, "rb");
    if (!fp) {
        printf("[Palette] 无法打开 %s\n", dat_file);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    dword file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    byte* file_data = (byte*)malloc(file_size);
    if (!file_data) {
        fclose(fp);
        return -1;
    }
    
    fread(file_data, 1, file_size, fp);
    fclose(fp);
    
    dword offsets[104];
    for (int i = 0; i < 104; i++) {
        offsets[i] = *(dword*)(file_data + 6 + i * 4);
    }
    
    dword palette_size = offsets[1] - offsets[0];
    if (palette_size >= 768) {
        palette_init_6bit(palette, file_data + offsets[0]);
    } else {
        printf("[Palette] 资源0大小不足: %d\n", palette_size);
        free(file_data);
        return -1;
    }
    
    free(file_data);
    return 0;
}

void palette_get_color(const Palette* palette, int index, byte out_rgb[3]) {
    if (index < 0 || index >= PALETTE_SIZE) {
        out_rgb[0] = out_rgb[1] = out_rgb[2] = 0;
        return;
    }
    out_rgb[0] = palette->colors[index][0];
    out_rgb[1] = palette->colors[index][1];
    out_rgb[2] = palette->colors[index][2];
}

Image* image_decode_bmp(const byte* data, int width, int height, const Palette* palette) {
    if (!data || width <= 0 || height <= 0 || !palette) return NULL;
    
    Image* img = (Image*)malloc(sizeof(Image));
    if (!img) return NULL;
    
    img->width = width;
    img->height = height;
    img->data = (byte*)malloc(width * height);
    
    if (!img->data) {
        free(img);
        return NULL;
    }
    
    int idx = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (idx < width * height) {
                img->data[idx] = data[idx];
                idx++;
            }
        }
    }
    
    return img;
}

Image* image_decode_face(const byte* data, int size, const Palette* palette) {
    if (!data || size < 4 || !palette) return NULL;
    
    int width = *(short*)(data + 0);
    int height = *(short*)(data + 2);
    
    if (width <= 0 || height <= 0 || width > 512 || height > 512) {
        return NULL;
    }
    
    Image* img = (Image*)malloc(sizeof(Image));
    if (!img) return NULL;
    
    img->width = width;
    img->height = height;
    img->data = (byte*)malloc(width * height);
    
    if (!img->data) {
        free(img);
        return NULL;
    }
    
    int src = 4;
    int end = size;
    int x = 0;
    int y = 0;
    int repeat = 1;
    bool waiting_for_repeat = true;
    
    while (src < end && y < height) {
        byte b = data[src++];
        
        if (b > 192 && waiting_for_repeat) {
            repeat = b - 192;
            waiting_for_repeat = false;
        } else {
            waiting_for_repeat = true;
            for (int i = 0; i < repeat && y < height; i++) {
                img->data[y * width + x] = b;
                x++;
                if (x >= width) {
                    x = 0;
                    y++;
                }
            }
            repeat = 1;
        }
    }
    
    return img;
}

Image* image_decode_bg(const byte* data, int size, const Palette* palette) {
    if (!data || size < 4 || !palette) return NULL;
    
    int width = *(short*)(data + 0);
    int height = *(short*)(data + 2);
    
    if (width <= 0 || height <= 0) {
        return NULL;
    }
    
    Image* img = (Image*)malloc(sizeof(Image));
    if (!img) return NULL;
    
    img->width = width;
    img->height = height;
    img->data = (byte*)malloc(width * height);
    
    if (!img->data) {
        free(img);
        return NULL;
    }
    
    int src = 4;
    int end = size;
    int x = 0;
    int y = 0;
    
    while (src < end && y < height) {
        bool flag = false;
        
        if (x != 0) {
            flag = true;
        }
        
        flag = (x != 0);
        
        byte b = data[src++];
        
        if (!flag) {
            img->data[y * width + x] = b;
            x++;
            if (x >= width) {
                x = 0;
                y++;
            }
        } else {
            int repeat = b & 0x3F;
            if (repeat == 0) continue;
            
            byte color = data[src++];
            
            for (int i = 0; i < repeat && y < height; i++) {
                img->data[y * width + x] = color;
                x++;
                if (x >= width) {
                    x = 0;
                    y++;
                }
            }
        }
    }
    
    return img;
}

void image_free(Image* img) {
    if (img) {
        if (img->data) free(img->data);
        free(img);
    }
}

void image_blit(byte* dst, int dst_w, int dst_h, int x, int y, int w, int h, const byte* src) {
    if (!dst || !src) return;
    
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            int px = x + dx;
            int py = y + dy;
            
            if (px >= 0 && px < dst_w && py >= 0 && py < dst_h) {
                dst[py * dst_w + px] = src[dy * w + dx];
            }
        }
    }
}

void image_fill(byte* img, int w, int h, byte color) {
    if (!img) return;
    memset(img, color, w * h);
}
