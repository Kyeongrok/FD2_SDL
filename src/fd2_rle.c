#include "fd2_rle.h"
#include <string.h>

/**
 * Decompress FDOTHER.DAT resources using the EXACT algorithm from makeShapBMP.
 * 
 * This is a line-based decompression where each "row" is processed separately.
 * The algorithm processes bytes one at a time with a flag-based state machine.
 * 
 * Variable mapping from Python to C:
 *   num5 (skip_count) - bytes to skip in x direction
 *   num6 (cmd_count)   - repeat/draw counter from previous command
 *   num7 (draw_count)  - number of pixels to draw in this iteration
 *   num8 (x)           - current x position
 *   num9 (y)           - current y position
 *   b                  - current byte value
 *   flag               - whether to draw (True if num6 > 0)
 */
int fd2_decode_fdother_resource(byte* src, int src_size, byte* dst, int width, int height) {
    int num = src_size - 1;
    int num2 = 0;
    int skip_count = 0;
    int cmd_count = 0;
    int draw_count = 0;
    int x = 0;
    int y = 0;
    byte b = 0;
    int flag = 0;
    
    while (num2 <= num && y < height) {
        if (skip_count != 0) {
            skip_count = 0;
            flag = 1;
        } else {
            flag = 0;
        }
        
        flag = (cmd_count != 0);
        
        if (!flag) {
            skip_count = 0;
            cmd_count = 0;
            draw_count = 0;
            
            if (num2 < src_size) {
                b = src[num2];
            }
            
            if (b >= 192) {
                skip_count = b - 192 + 1;
            } else if (b >= 128) {
                cmd_count = b - 128 + 1;
            } else if (b >= 64) {
                draw_count = b - 64;
                cmd_count = 1;
                flag = 1;
            } else {
                cmd_count = 1;
                draw_count = b;
            }
            
            x += skip_count;
            if (x >= width) {
                x = 0;
                y++;
                flag = 0;
            }
        } else {
            int num10 = draw_count;
            int num11 = 0;
            
            while (1) {
                if (num11 > num10) {
                    break;
                }
                if (b >= 64 && b < 128) {
                    x++;
                }
                if (y < height && x < width) {
                    dst[y * width + x] = b;
                }
                x++;
                if (x >= width) {
                    x = 0;
                    y++;
                    flag = 0;
                }
                num11++;
            }
            cmd_count--;
        }
        
        num2++;
    }
    
    return 0;
}
