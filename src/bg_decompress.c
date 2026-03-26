/*
 * BG.DAT Battle Background Decompression
 * 
 * Based on IDA reverse engineering of FD2.EXE
 * Compatible with DOS game architecture - writes directly to buffer
 * 
 * Algorithm from sub_4E98D (makeBgBMP):
 * - Flag-based state machine
 * - Reads color index from stream during draw phase
 * - Supports skip, fill, and copy modes
 */

#include "../include/fd2_types.h"
#include <string.h>
#include <stdlib.h>

/* BG图像头 */
typedef struct {
    int16_t width;
    int16_t height;
} BGHeader;

/* 
 * 解压BG资源到像素缓冲区
 * 
 * 缓冲区布局: width * height字节，每字节是调色板索引
 * 
 * RLE命令类型:
 *   b >= 192:       跳过 (b & 0x3F) + 1 像素
 *   128 <= b < 192: 重复模式，num8 = (b & 0x3F) + 1
 *   64 <= b < 128:  连续模式，额外x增量
 *   b < 64:         填充模式
 * 
 * 在绘制模式下，从src当前位置读取颜色索引
 */
int bg_decompress(byte *src, int src_len, byte *dst, int stride) {
    if (!src || !dst || src_len < 4) return -1;
    
    /* 读取图像头 */
    BGHeader *hdr = (BGHeader *)src;
    int width = hdr->width;
    int height = hdr->height;
    
    if (width <= 0 || height <= 0 || width > 640 || height > 480) {
        return -1;
    }
    
    /* 初始化状态变量 */
    int src_pos = 4;      /* 跳过头 */
    int src_end = src_len - 1;
    
    int num7 = 0;         /* 跳过计数 */
    int num8 = 0;         /* 重复/命令计数 */
    int num9 = 0;         /* 绘制计数 */
    byte b = 0;           /* 当前命令字节 */
    int x = 0;            /* 当前X位置 */
    int y = 0;            /* 当前Y位置 */
    
    /* 清空目标缓冲区 */
    memset(dst, 0, stride * height);
    
    /* 主解压循环 */
    while (src_pos <= src_end && y < height) {
        int flag = (num8 != 0);
        
        if (!flag) {
            /* 读取新命令字节 */
            num7 = 0;
            num8 = 0;
            num9 = 0;
            
            if (src_pos < src_len) {
                b = src[src_pos];
                
                if (b >= 192) {
                    num7 = (b & 0x3F) + 1;
                }
                if (b >= 128 && b < 192) {
                    num8 = (b & 0x3F) + 1;
                }
                if (b >= 64 && b < 128) {
                    num9 = b - 64;
                    num8 = 1;
                }
                if (b < 64) {
                    num8 = 1;
                    num9 = b;
                }
            }
            
            /* 应用跳过 */
            x += num7;
            while (x >= width) {
                x -= width;
                y++;
            }
        } else {
            /* 绘制模式 */
            int count = num9;
            
            for (int i = 0; i <= count; i++) {
                /* 64-127范围需要额外x增量 */
                if (b >= 64 && b < 128) {
                    x++;
                    if (x >= width) {
                        x = 0;
                        y++;
                        if (y >= height) break;
                    }
                }
                
                /* 从src读取颜色索引 */
                if (src_pos < src_len) {
                    byte color_idx = src[src_pos];
                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        dst[y * stride + x] = color_idx;
                    }
                }
                
                x++;
                if (x >= width) {
                    x = 0;
                    y++;
                    if (y >= height) break;
                }
            }
            num8--;
        }
        
        src_pos++;
    }
    
    return 0;
}

/* 
 * 渲染BG图像到RGB缓冲区
 * 
 * src: 原始BG.DAT资源数据（包含4字节头）
 * src_len: 数据长度
 * palette: 256色RGB调色板 (768字节)
 * dst: 输出RGB缓冲区 (width * height * 3)
 * dst_width: 输出宽度
 * dst_height: 输出高度
 */
int bg_render_to_rgb(byte *src, int src_len, 
                     const byte *palette,
                     byte *dst, int dst_width, int dst_height) {
    if (!src || !dst || !palette || src_len < 4) return -1;
    
    /* 读取图像尺寸 */
    int16_t img_width = src[0] | (src[1] << 8);
    int16_t img_height = src[2] | (src[3] << 8);
    
    if (img_width <= 0 || img_height <= 0) return -1;
    
    /* 确保输出尺寸匹配 */
    if (img_width > dst_width || img_height > dst_height) {
        return -1;
    }
    
    /* 分配临时缓冲区用于像素索引 */
    byte *indices = (byte *)malloc(img_width * img_height);
    if (!indices) return -1;
    
    /* 解压到索引缓冲区 */
    int result = bg_decompress(src, src_len, indices, img_width);
    if (result != 0) {
        free(indices);
        return -1;
    }
    
    /* 转换为RGB */
    for (int y = 0; y < img_height; y++) {
        for (int x = 0; x < img_width; x++) {
            byte idx = indices[y * img_width + x];
            int dst_idx = (y * dst_width + x) * 3;
            dst[dst_idx + 0] = palette[idx * 3 + 0];  /* R */
            dst[dst_idx + 1] = palette[idx * 3 + 1];  /* G */
            dst[dst_idx + 2] = palette[idx * 3 + 2];  /* B */
        }
    }
    
    free(indices);
    return 0;
}

/* 获取BG图像尺寸 */
int bg_get_dimensions(byte *src, int src_len, int *width, int *height) {
    if (!src || src_len < 4 || !width || !height) return -1;
    
    *width = src[0] | (src[1] << 8);
    *height = src[2] | (src[3] << 8);
    
    return 0;
}
