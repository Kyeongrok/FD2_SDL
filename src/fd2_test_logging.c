#include "../include/fd2_test_logging.h"
#include <stdlib.h>
#include <string.h>

static FILE* g_json = NULL;
static int g_palette_entries = 0;
static int g_bmp4x4_done = 0;

void test_log_json_start(void) {
    g_json = fopen("test_result.json", "w");
    if (!g_json) return;
    fprintf(g_json, "{\n  \"test\": \"FD2 图像模块扩展测试\",\n");
    fprintf(g_json, "  \"palette\": [");
    g_palette_entries = 0;
}

void test_log_json_end(void) {
    if (g_json) {
        fprintf(g_json, "\n  ],\n  \"bmp4x4\": []\n}\n");
        fclose(g_json);
        g_json = NULL;
    }
}

void test_log_json_palette_sample(int idx, const byte rgb[3]) {
    if (!g_json) return;
    if (g_palette_entries > 0) fprintf(g_json, ",");
    fprintf(g_json, "{\"idx\":%d,\"rgb\":[%u,%u,%u]}", idx, rgb[0], rgb[1], rgb[2]);
    g_palette_entries++;
}

void test_log_json_bmp4x4(const unsigned char bmp[4][4][3]) {
    if (!g_json) return;
    fprintf(g_json, ",\n  \"bmp4x4\": [");
    for (int y = 0; y < 4; y++) {
        fprintf(g_json, "[");
        for (int x = 0; x < 4; x++) {
            fprintf(g_json, "[%u,%u,%u]", bmp[y][x][0], bmp[y][x][1], bmp[y][x][2]);
            if (x < 3) fprintf(g_json, ",");
        }
        fprintf(g_json, "]");
        if (y < 3) fprintf(g_json, ",");
    }
    fprintf(g_json, "]");
    g_bmp4x4_done = 1;
}

void test_log_json_face2x2(const unsigned char rgb2x2[2][2][3]) {
    if (!g_json) return;
    fprintf(g_json, ",\n  \"face2x2\": [[[%u,%u,%u],[%u,%u,%u]],[[%u,%u,%u],[%u,%u,%u]]]", 
        rgb2x2[0][0][0], rgb2x2[0][0][1], rgb2x2[0][0][2], rgb2x2[0][1][0], rgb2x2[0][1][1], rgb2x2[0][1][2],
        rgb2x2[1][0][0], rgb2x2[1][0][1], rgb2x2[1][0][2], rgb2x2[1][1][0], rgb2x2[1][1][1], rgb2x2[1][1][2]);
}

void test_log_json_bg2x2(const unsigned char rgb2x2[2][2][3]) {
    if (!g_json) return;
    fprintf(g_json, ",\n  \"bg2x2\": [[[%u,%u,%u],[%u,%u,%u]],[[%u,%u,%u],[%u,%u,%u]]]", 
        rgb2x2[0][0][0], rgb2x2[0][0][1], rgb2x2[0][0][2], rgb2x2[0][1][0], rgb2x2[0][1][1], rgb2x2[0][1][2],
        rgb2x2[1][0][0], rgb2x2[1][0][1], rgb2x2[1][0][2], rgb2x2[1][1][0], rgb2x2[1][1][1], rgb2x2[1][1][2]);
}
