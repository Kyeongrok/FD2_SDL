#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

byte ani_palette_buf[768];
byte ani_screen_buf[64000];

void do_h0(byte* d) { byte b=d[0]; dword v=(b<<16)|(b<<8)|b; dword* p=(dword*)ani_palette_buf; for(int i=0;i<96;i++)p[i]=v; }
void do_h1(byte* d, int sz) { if(sz>768)sz=768; memcpy(ani_palette_buf,d,sz); }
void do_h2(byte* d, int sz) {
    byte* dst=ani_palette_buf; int filled=0,pos=0;
    while(filled<768&&pos<sz){byte b=d[pos++];if((b&0xC0)==0xC0){int run=b&0x3F;byte v=d[pos++];for(int j=0;j<run&&filled<768;j++){*dst++=v;filled++;}}else{*dst++=b;filled++;}}
}
void do_h3(byte* d, int sz) {
    int pos=0; if(pos>=sz)return;
    int count=d[pos++]; byte* dst=ani_palette_buf;
    for(int i=0;i<count&&pos+4<=sz;i++){int olo=d[pos++],ohi=d[pos++];int off=olo|(ohi<<8);int clo=d[pos++],chi=d[pos++];int cp=clo|(chi<<8);if(off>=0&&cp>0&&off+cp<=768&&pos+cp<=sz){memcpy(dst+off,d+pos,cp);}pos+=cp;}
}
void do_h4(byte* d) { byte b=d[0]; dword v=(b<<16)|(b<<8)|b; dword* p=(dword*)ani_screen_buf; for(int i=0;i<10000;i++)p[i]=v; for(int i=40000;i<64000;i++)ani_screen_buf[i]=b; }
void do_h5(byte* d) { memcpy(ani_screen_buf,d,64000); }
void do_h6(byte* d, int sz) {
    byte* dst=ani_screen_buf; int filled=0,pos=0;
    while(filled<64000&&pos<sz){byte b=d[pos++];if((b&0xC0)==0xC0){int run=b&0x3F;byte v=d[pos++];for(int j=0;j<run&&filled<64000;j++){*dst++=v;filled++;}}else{*dst++=b;filled++;}}
}
void do_h7(byte* d, int sz) {
    int pos=0; if(pos+2>sz)return;
    int lo=d[pos++],hi=d[pos++]; int count=lo|(hi<<8);
    for(int i=0;i<count&&pos+4<=sz;i++){int olo=d[pos++],ohi=d[pos++];int off=olo|(ohi<<8);int plo=d[pos++],phi=d[pos++];int pi=plo|(phi<<8);if(off>=0&&off<64000)ani_screen_buf[off]=(byte)pi;}
}
void do_h8(byte* d, int sz) {
    int pos=0; if(pos+2>sz)return;
    int lo=d[pos++],hi=d[pos++]; int count=lo|(hi<<8);
    for(int i=0;i<count&&pos+5<=sz;i++){int olo=d[pos++],ohi=d[pos++];int off=olo|(ohi<<8);int slo=d[pos++],shi=d[pos++];int stride=slo|(shi<<8);byte v=d[pos++];for(int j=0;j<stride&&off+j<64000;j++)ani_screen_buf[off+j]=v;}
}
void do_h9(byte* d, int sz) {
    int pos=0; if(pos+2>sz)return;
    int lo=d[pos++],hi=d[pos++]; int count=lo|(hi<<8);
    for(int i=0;i<count&&pos+4<=sz;i++){int olo=d[pos++],ohi=d[pos++];int off=olo|(ohi<<8);int slo=d[pos++],shi=d[pos++];int stride=slo|(shi<<8);if(off>=0&&stride>0&&off+stride<=64000&&pos+stride<=sz)memcpy(ani_screen_buf+off,d+pos,stride);pos+=stride;}
}

void run_cmd(int cmd, byte* data, int pos, int sz) {
    int rem = sz - pos;
    switch(cmd) {
        case 0: do_h0(data+pos); break;
        case 1: do_h1(data+pos, rem); break;
        case 2: do_h2(data+pos, rem); break;
        case 3: do_h3(data+pos, rem); break;
        case 4: do_h4(data+pos); break;
        case 5: do_h5(data+pos); break;
        case 6: do_h6(data+pos, rem); break;
        case 7: do_h7(data+pos, rem); break;
        case 8: do_h8(data+pos, rem); break;
        case 9: do_h9(data+pos, rem); break;
    }
}

int main() {
    FILE* fp = fopen("ANI.DAT", "rb");
    if (!fp) { printf("Can't open ANI.DAT\n"); return 1; }
    
    fseek(fp, 6, SEEK_SET);
    dword offset; fread(&offset, 4, 1, fp);
    printf("Resource 0 offset: 0x%X\n", offset);
    
    fseek(fp, offset, SEEK_SET);
    byte header[165];
    fread(header, 1, 165, fp);
    
    word block_count = *(word*)(header + 165);
    printf("Block count: %d\n", block_count);
    
    // CORRECT: block header at offset+167, data at offset+175
    int file_pos = offset + 167;
    
    for (int i = 0; i < 3; i++) {
        byte bh[8]; fseek(fp, file_pos, SEEK_SET);
        fread(bh, 1, 8, fp);
        word size = *(word*)(bh+0);
        word cmds = *(word*)(bh+2);
        printf("Block %d: header at 0x%X, size=%d, cmds=%d\n", i, file_pos, size, cmds);
        
        byte* bd = malloc(size);
        fread(bd, 1, size, fp);
        
        int pos = 0;
        int exec = 0;
        for (int c = 0; c < cmds && pos < size; c++) {
            byte cmd = bd[pos++];
            if (cmd < 10) {
                run_cmd(cmd, bd, pos, size);
                exec++;
            }
        }
        printf("  Executed %d commands\n", exec);
        free(bd);
        file_pos += size;
    }
    
    printf("\nPalette[0..9] RGB: ");
    for (int i = 0; i < 10; i++)
        printf("(%d,%d,%d) ", ani_palette_buf[i*3], ani_palette_buf[i*3+1], ani_palette_buf[i*3+2]);
    printf("\n");
    
    int nonzero = 0;
    for (int i = 0; i < 768; i++) if (ani_palette_buf[i]) nonzero++;
    printf("Non-zero palette bytes: %d / 768\n", nonzero);
    
    int nonzero_screen = 0;
    for (int i = 0; i < 64000; i++) if (ani_screen_buf[i]) nonzero_screen++;
    printf("Non-zero screen bytes: %d / 64000\n", nonzero_screen);
    
    fclose(fp);
    return 0;
}
