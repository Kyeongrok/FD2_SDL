#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;
typedef unsigned short word;

byte palette[768];

void decompress_rle(byte* src, int src_size, byte* dst, int width, int height, int stride) {
    if (src_size < 4) return;
    
    word count = *(word*)src;
    word num_rows = *(word*)(src + 2);
    
    src += 4;
    src_size -= 4;
    
    int row_gap = stride - count;
    int pos = 0;
    
    for (int row = 0; row < num_rows && row < height; row++) {
        int remaining = count;
        
        while (remaining > 0 && pos < src_size) {
            if (pos >= src_size) break;
            byte code = src[pos++];
            
            int num = (code & 0x3F) + 1;
            if (num > remaining) num = remaining;
            
            int type = (code >> 6) & 0x3;
            
            if (type != 3) {
                if (pos < src_size) {
                    byte fill = src[pos++];
                    memset(dst, fill, num);
                    dst += num;
                }
            } else {
                memset(dst, code, num);
                dst += num;
            }
            remaining -= num;
        }
        
        dst += row_gap;
    }
}

void flip_vertical(byte* data, int width, int height) {
    byte* temp = (byte*)malloc(width);
    for (int y = 0; y < height / 2; y++) {
        byte* top = data + y * width;
        byte* bottom = data + (height - 1 - y) * width;
        memcpy(temp, top, width);
        memcpy(top, bottom, width);
        memcpy(bottom, temp, width);
    }
    free(temp);
}

void save_bmp(byte* indexed, int width, int height, const char* fname) {
    FILE* f = fopen(fname, "wb");
    if (!f) return;
    
    int data_size = width * height * 3;
    int file_size = 54 + data_size;
    
    fputc('B', f); fputc('M', f);
    fwrite(&file_size, 4, 1, f);
    int zero = 0;
    fwrite(&zero, 2, 1, f);
    fwrite(&zero, 2, 1, f);
    int offset = 54;
    fwrite(&offset, 4, 1, f);
    
    int dib = 40;
    fwrite(&dib, 4, 1, f);
    fwrite(&width, 4, 1, f);
    fwrite(&height, 4, 1, f);
    short planes = 1;
    fwrite(&planes, 2, 1, f);
    short bpp = 24;
    fwrite(&bpp, 2, 1, f);
    fwrite(&zero, 4, 1, f);
    fwrite(&data_size, 4, 1, f);
    int ppm = 2835;
    fwrite(&ppm, 4, 1, f);
    fwrite(&ppm, 4, 1, f);
    fwrite(&zero, 4, 1, f);
    fwrite(&zero, 4, 1, f);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            byte idx = indexed[y * width + x];
            byte r = palette[idx * 3];
            byte g = palette[idx * 3 + 1];
            byte b = palette[idx * 3 + 2];
            fputc(b, f);
            fputc(g, f);
            fputc(r, f);
        }
    }
    fclose(f);
}

int main() {
    FILE* fp = fopen("data/fd2/FDOTHER.DAT", "rb");
    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    byte* data = (byte*)malloc(fsize);
    fread(data, 1, fsize, fp);
    fclose(fp);
    
    byte* pal_src = data + 422;
    for (int i = 0; i < 256; i++) {
        palette[i*3] = (pal_src[i*3] << 2) | (pal_src[i*3] >> 4);
        palette[i*3+1] = (pal_src[i*3+1] << 2) | (pal_src[i*3+1] >> 4);
        palette[i*3+2] = (pal_src[i*3+2] << 2) | (pal_src[i*3+2] >> 4);
    }
    
    int offsets[200];
    int count = 0, pos = 6;
    while (pos + 4 <= fsize && count < 200) {
        offsets[count++] = *(int*)(data + pos);
        pos += 4;
    }
    
    int resources[] = {11, 15, 55, 56, 59, 60, 61, 62, 74, 75, 97, 100};
    
    for (int i = 0; i < sizeof(resources)/sizeof(resources[0]); i++) {
        int r = resources[i];
        int start = offsets[r];
        int sz = offsets[r+1] - start;
        
        printf("Resource %d: size=%d\n", r, sz);
        
        byte decoded[64000];
        memset(decoded, 0, 64000);
        
        if (sz == 64004) {
            memcpy(decoded, data + start + 4, 64000);
        } else {
            decompress_rle(data + start, sz, decoded, 320, 200, 320);
        }
        
        flip_vertical(decoded, 320, 200);
        
        char fname[64];
        sprintf(fname, "images/res_%02d_final2.bmp", r);
        save_bmp(decoded, 320, 200, fname);
        
        int nz = 0;
        for (int j = 0; j < 64000; j++) if (decoded[j]) nz++;
        printf("  Non-zero: %d\n", nz);
    }
    
    free(data);
    return 0;
}
