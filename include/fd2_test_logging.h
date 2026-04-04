#ifndef FD2_TEST_LOGGING_H
#define FD2_TEST_LOGGING_H

#include "fd2_image.h" // 需要 Palette/byte 类型
#include <stdio.h>

// 简单 JSON 日志输出接口，供测试用
void test_log_json_start(void);
void test_log_json_end(void);
void test_log_json_palette_sample(int idx, const byte rgb[3]);
void test_log_json_bmp4x4(const unsigned char bmp[4][4][3]);
void test_log_json_face2x2(const unsigned char rgb2x2[2][2][3]);
void test_log_json_bg2x2(const unsigned char rgb2x2[2][2][3]);

#endif
