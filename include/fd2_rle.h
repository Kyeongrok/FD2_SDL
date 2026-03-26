#ifndef FD2_RLE_H
#define FD2_RLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint8_t byte;
typedef uint16_t word;

/**
 * Decompress FDOTHER.DAT resources using the makeShapBMP algorithm.
 * 
 * @param src      Source compressed data (including 4-byte width/height header)
 * @param src_size Size of source data
 * @param dst      Destination buffer (must be width * height bytes)
 * @param width    Image width
 * @param height   Image height
 * @return 0 on success, -1 on error
 */
int fd2_decode_fdother_resource(byte* src, int src_size, byte* dst, int width, int height);

/**
 * Decompress BG.DAT battle background images.
 * 
 * Algorithm matches IDA function sub_4E98D and base_parser.py makeBgBMP.
 * 
 * RLE command byte format:
 *   b >= 192:       Skip (b - 192 + 1) pixels (transparent)
 *   128 <= b < 192: Fill (b - 128 + 1) pixels with next byte value
 *   64 <= b < 128:  Copy (b - 64 + 1) bytes from source
 *   b < 64:         Fill (b + 1) pixels with next byte value
 * 
 * @param src      Source data pointer (includes 4-byte width/height header)
 * @param length   Length of source data
 * @param palette  256-color palette (RGB format, 768 bytes)
 * @param dst      Destination buffer (for pixel indices, width * height bytes)
 * @param stride   Destination stride (line width in bytes)
 * @return 0 on success, -1 on error
 */
int fd2_decode_bg_resource(byte* src, int length, byte* palette, byte* dst, int stride);

#ifdef __cplusplus
}
#endif

#endif
