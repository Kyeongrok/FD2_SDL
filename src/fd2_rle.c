#include "../include/fd2_rle.h"
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

/**
 * Decompress BG.DAT battle background images.
 * 
 * This implements the RLE algorithm from IDA sub_4E98D (makeBgBMP).
 * 
 * The algorithm uses a flag-based state machine:
 * - When flag=0: Read a command byte and set up parameters
 * - When flag=1: Draw pixels using the current parameters
 * 
 * Command byte interpretation:
 *   b >= 192 (11xxxxxx): Skip mode - advance dst by (b & 0x3F) + 1
 *   128-191 (10xxxxxx):  Fill mode - set cmd_count, draw (b & 0x3F) + 1 pixels
 *   64-127 (01xxxxxx):   Copy mode - set cmd_count=1, draw_count=(b & 0x3F)
 *   0-63 (00xxxxxx):     Fill mode - set cmd_count=1, draw_count=b
 * 
 * In draw mode, the next byte in the stream (at current src position) is used
 * as the color index for the pixels being drawn.
 */
int fd2_decode_bg_resource(byte* src, int length, byte* palette, byte* dst, int stride) {
    if (!src || !dst || length < 4) {
        return -1;
    }
    
    // Read width and height from header
    int16_t width = (int16_t)(src[0] | (src[1] << 8));
    int16_t height = (int16_t)(src[2] | (src[3] << 8));
    
    if (width <= 0 || height <= 0) {
        return -1;
    }
    
    int src_pos = 4;  // Start after header
    int src_end = length - 1;
    
    int num7 = 0;  // skip count
    int num8 = 0;  // cmd/repeat count
    int num9 = 0;  // draw count
    byte b = 0;    // current command byte
    int x = 0;     // current x position
    int y = 0;     // current y position
    
    while (src_pos <= src_end && y < height) {
        int flag = (num8 != 0);
        
        if (!flag) {
            // Read new command byte
            num7 = 0;
            num8 = 0;
            num9 = 0;
            
            if (src_pos < length) {
                b = src[src_pos];
                
                if (b >= 192) {
                    num7 = b - 192 + 1;
                }
                if (b >= 128 && b < 192) {
                    num8 = b - 128 + 1;
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
            
            x += num7;
            if (x >= width) {
                x = 0;
                y++;
                flag = 0;
            }
        } else {
            // Draw mode
            int count = num9;
            
            for (int i = 0; i <= count; i++) {
                // Extra x increment for copy mode (64-127)
                if (b >= 64 && b < 128) {
                    x++;
                }
                
                // Get color index from current src position
                if (src_pos < length) {
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
