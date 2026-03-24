#ifndef FD2_TYPES_H
#define FD2_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define PALETTE_SIZE 256

typedef struct {
    byte* data;
    dword size;
    int resource_count;
    int max_resources;
    dword* starts;
    dword* ends;
} DatHandle;

#endif
