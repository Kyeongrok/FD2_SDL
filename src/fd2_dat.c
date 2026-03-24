/*
 * FD2 Game - Common DAT File Loading Functions
 * Based on reverse engineering of FD2.EXE
 * 
 * Key functions:
 * - fd_load_resource(): Load resource from DAT file (like sub_111BA)
 * - fd_decompress_rle(): RLE decompression (like sub_4E98D)
 * - fd_get_palette(): Load and expand palette
 */

#include "../include/fd2_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* DAT file header sizes */
#define DAT_HEADER_SIZE 6
#define OFFSET_ENTRY_SIZE 4
#define MAX_RESOURCES 150

/* VRAM base address forega DOS graphics */
#define VRAM_BASE 0xA0000

/* Load palette from resource 0 of FDOTHER.DAT */
/* Expands 6-bit palette (0-63) to 8-bit (0-255) */
int fd_load_palette(const char *filename, byte palette[768]) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error: Cannot open %s\n", filename);
        return -1;
    }
    
    /* Read 6-byte header */
    byte header[6];
    fread(header, 1, 6, fp);
    
    /* Read offset table - skip first entry (palette is at index 0) */
    dword offsets[MAX_RESOURCES];
    for (int i = 0; i < MAX_RESOURCES; i++) {
        byte entry[4];
        if (fread(entry, 1, 4, fp) != 4) break;
        offsets[i] = entry[0] | (entry[1] << 8) | (entry[2] << 16) | (entry[3] << 24);
    }
    
    /* Resource 0 is palette - read 768 bytes */
    fseek(fp, offsets[0], SEEK_SET);
    byte palette_data[768];
    fread(palette_data, 1, 768, fp);
    fclose(fp);
    
    /* Expand 6-bit to 8-bit (multiply by 4) */
    for (int i = 0; i < 256; i++) {
        palette[i * 3]     = palette_data[i * 3] * 4;
        palette[i * 3 + 1] = palette_data[i * 3 + 1] * 4;
        palette[i * 3 + 2] = palette_data[i * 3 + 2] * 4;
    }
    
    return 0;
}

/* Load resource from FDOTHER.DAT - equivalent to sub_111BA
 * 
 * a1 = filename
 * a2 = previous buffer (to free)
 * a3 = resource index
 * returns: allocated buffer with resource data
 */
byte* fd2_load_dat_resource(const char *filename, byte *prev_buf, int resource_idx, dword *out_size) {
    if (prev_buf) {
        free(prev_buf);
    }
    
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("\n\n File not found %s!!! \n\n", filename);
        return NULL;
    }
    
    byte header[8];
    fseek(fp, 4 * resource_idx + 6, SEEK_SET);
    if (fread(header, 1, 8, fp) != 8) {
        fclose(fp);
        return NULL;
    }
    
    dword offset = *(dword*)header;
    dword end_offset = *(dword*)(header + 4);
    dword data_size = end_offset - offset;
    
    byte *data = (byte*)malloc(data_size);
    if (!data) {
        printf("Out of Memory at Load %s Number:%d!!\n", filename, resource_idx);
        fclose(fp);
        return NULL;
    }
    
    fseek(fp, offset, SEEK_SET);
    fread(data, 1, data_size, fp);
    fclose(fp);
    
    if (out_size) *out_size = data_size;
    
    return data;
}

/* Get image dimensions from resource header */
void fd_get_image_dimensions(const byte *data, int *width, int *height) {
    if (!data) return;
    *width = data[0] | (data[1] << 8);
    *height = data[2] | (data[3] << 8);
}

/* RLE Decompression - Based on sub_4E98D
 * 
 * Parameters:
 *   src: compressed data
 *   src_size: size of compressed data
 *   dst: output buffer
 *   dst_width: output width
 *   dst_height: output height
 *   value_param: -1 for default mode (most common)
 *
 * Type decoding based on code byte:
 *   bit7=0, bit6=0: Type 0 - Fill with next byte
 *   bit7=0, bit6=1: Type 1 - Literal copy  
 *   bit7=1, bit6=0: Type 2 - Literal (with double decrement)
 *   bit7=1: Check bit5:
 *            bit5=0: Type 3a - Literal (like Type 1)
 *            bit5=1: Type 3b - Skip (just advance position)
 *
 * The count formula from IDA:
 *   count = ((code & 0x3F) >> 2) + 1
 */
int fd_decompress_rle(const byte *src, int src_size, 
                       byte *dst, int dst_width, int dst_height,
                       int value_param) {
    if (!src || !dst || src_size < 4) return -1;
    
    /* Read header: first 4 bytes are width and height */
    int width = src[0] | (src[1] << 8);
    int height = src[2] | (src[3] << 8);
    
    /* Use provided dimensions if header is invalid */
    if (width <= 0 || height <= 0) {
        width = dst_width;
        height = dst_height;
    }
    
    /* Starting position in compressed data */
    int i = 4;
    int row = 0;
    int col = 0;
    
    while (i < src_size && row < height && row < 200) {
        byte code = src[i];
        byte bit7 = (code >> 7) & 1;
        byte bit6 = (code >> 6) & 1;
        byte bit5 = (code >> 5) & 1;
        
        /* Count is extracted from lower bits */
        int count = ((code & 0x3F) >> 2) + 1;
        
        /* Type 0: bit7=0, bit6=0 - Fill with next byte */
        if (bit7 == 0 && bit6 == 0) {
            /* count = ((code >> 2) & 0x0F) + 1 */
            count = ((code >> 2) & 0x0F) + 1;
            byte fill = (i + 1 < src_size) ? src[i + 1] : 0;
            for (int j = 0; j < count; j++) {
                if (row < height && col < width) {
                    dst[row * width + col] = fill;
                }
                col++;
                if (col >= width) {
                    col = 0;
                    row++;
                }
            }
            i += 2;
        }
        /* Type 1: bit7=0, bit6=1 - Literal copy */
        else if (bit7 == 0 && bit6 == 1) {
            /* count = ((code >> 2) & 0x0F) + 1 */
            count = ((code >> 2) & 0x0F) + 1;
            for (int j = 0; j < count; j++) {
                if (row < height && col < width && i + 1 + j < src_size) {
                    dst[row * width + col] = src[i + 1 + j];
                }
                col++;
                if (col >= width) {
                    col = 0;
                    row++;
                }
            }
            i += 1 + count;
        }
        /* Type 2: bit7=1, bit6=0 - Double skip then literal */
        else if (bit7 == 1 && bit6 == 0) {
            /* count = ((code >> 2) & 0x0F) + 1, but if code >= 0x40 then count=1 */
            count = ((code >> 2) & 0x0F) + 1;
            /* Skip count pixels twice */
            for (int j = 0; j < count; j++) {
                col++;
                if (col >= width) { col = 0; row++; }
            }
            /* Write literal */
            for (int j = 0; j < count; j++) {
                if (row < height && col < width && i + 1 + j < src_size) {
                    dst[row * width + col] = src[i + 1 + j];
                }
                col++;
                if (col >= width) { col = 0; row++; }
            }
            i += 1 + count;
        }
        /* Type 3: bit7=1 - check bit5 */
        else {
            /* count = ((code & 0x3F) >> 2) + 1 for bit7=bit6=1 case */
            if (bit5 == 0) {
                /* Type 3a - Literal like Type 1 */
                count = ((code >> 2) & 0x0F) + 1;
                for (int j = 0; j < count; j++) {
                    if (row < height && col < width && i + 1 + j < src_size) {
                        dst[row * width + col] = src[i + 1 + j];
                    }
                    col++;
                    if (col >= width) { col = 0; row++; }
                }
                i += 1 + count;
            } else {
                /* Type 3b - Skip (advance position without writing) */
                count = ((code & 0x3F) >> 2) + 1;
                for (int j = 0; j < count; j++) {
                    col++;
                    if (col >= width) { col = 0; row++; }
                }
                i += 1;
            }
        }
    }
    
    return 0;
}

/* Parse and analyze a resource - for debugging */
int fd_analyze_resource(const byte *data, int size) {
    if (!data || size < 4) return -1;
    
    printf("Resource Analysis:\n");
    printf("  Size: %d bytes\n", size);
    printf("  Header: %02X %02X %02X %02X\n", 
           data[0], data[1], data[2], data[3]);
    
    int width = data[0] | (data[1] << 8);
    int height = data[2] | (data[3] << 8);
    printf("  Dimensions: %dx%d\n", width, height);
    
    /* Count byte types in compressed data */
    int type0 = 0, type1 = 0, type2 = 0, type3a = 0, type3b = 0;
    int zero_count = 0;
    
    for (int i = 4; i < size; i++) {
        byte code = data[i];
        byte bit7 = (code >> 7) & 1;
        byte bit6 = (code >> 6) & 1;
        byte bit5 = (code >> 5) & 1;
        
        if (code == 0) zero_count++;
        else if (bit7 == 0 && bit6 == 0) type0++;
        else if (bit7 == 0 && bit6 == 1) type1++;
        else if (bit7 == 1 && bit6 == 0) type2++;
        else if (bit5 == 0) type3a++;
        else type3b++;
    }
    
    printf("  Type counts (from %d bytes):\n", size - 4);
    printf("    Type 0 (fill): %d\n", type0);
    printf("    Type 1 (literal): %d\n", type1);
    printf("    Type 2 (double-skip lit): %d\n", type2);
    printf("    Type 3a (literal): %d\n", type3a);
    printf("    Type 3b (skip): %d\n", type3b);
    printf("    Zero bytes: %d\n", zero_count);
    
    return 0;
}