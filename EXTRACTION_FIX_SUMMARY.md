# FDOTHER.DAT Extraction Fix Summary

## Problem Identified
Images extracted from FDOTHER.DAT were showing "horizontal lines" patterns instead of proper game graphics.

## Root Cause
Two issues were identified:

### 1. Offset Table Format
The code was incorrectly parsing the offset table entries:
- **Correct format**: 4-byte entries (sequential offsets)
- **Incorrect assumption**: Some code assumed 8-byte [offset, next_offset] pairs

The 4-byte format correctly produces:
- Resource 0: offset=422, size=768 (palette)
- Resource 1: offset=1190, size=2235
- Resource 2: offset=3425, size=37680

### 2. RLE Decompression Logic
The original decompression had issues with the stride handling. The correct implementation:

```c
for (int row = 0; row < num_rows; row++) {
    int remaining = count;
    
    while (remaining > 0) {
        byte val = *src++;
        int num = (val & 0x3F) + 1;
        
        if ((val & 0x80) == 0) {
            if ((val & 0x40) == 0) {
                // 00xxxxxx: literal copy
                memcpy(dst_ptr, src, num);
                src += num;
            } else {
                // 01xxxxxx: fill with 0
                memset(dst_ptr, 0, num);
            }
        } else if ((val & 0x40) == 0) {
            // 10xxxxxx: single-byte repeat
            byte fill = *src++;
            for (int i = 0; i < num; i++) *dst_ptr++ = fill;
        } else {
            // 11xxxxxx: skip (write same byte)
            byte fill = *src++;
            for (int i = 0; i < num; i++) *dst_ptr++ = fill;
        }
        remaining -= num;
    }
    
    dst_row += dst_stride;
}
```

## Results
12 320x200 images successfully extracted:

| Resource | Size | Type | Colors | Content Rows |
|----------|------|------|--------|--------------|
| 11 | 53587 | Compressed (83.7%) | 114 | 183/200 |
| 15 | 64004 | Raw | 15 | 200/200 |
| 55 | 64004 | Raw | 16 | 200/200 |
| 56 | 13609 | Compressed (21.3%) | 68 | 197/200 |
| 59 | 3278 | Compressed (5.1%) | 73 | 183/200 |
| 60 | 2849 | Compressed (4.5%) | 68 | 182/200 |
| 61 | 54727 | Compressed (85.5%) | 126 | 188/200 |
| 62 | 43434 | Compressed (67.9%) | 113 | 182/200 |
| 74 | 6585 | Compressed (10.3%) | 126 | 180/200 |
| 75 | 37776 | Compressed (59.0%) | 166 | 100/200 |
| 97 | 39358 | Compressed (61.5%) | 42 | 199/200 |
| 100 | 15746 | Compressed (24.6%) | 107 | 182/200 |

## Files Updated
- `src/fd2_rle.c` - Updated RLE decompression function
- `src/fdother_extract.c` - Working standalone extraction tool
- `images/*.bmp` and `images/*.png` - Extracted images

## Verification
All extracted images have:
- Correct 320x200 dimensions
- Varied color palettes (15-166 unique colors)
- Multiple rows with content (not just horizontal lines)
- Valid BMP format (192054 bytes per image)
