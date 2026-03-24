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

#ifdef __cplusplus
}
#endif

#endif
