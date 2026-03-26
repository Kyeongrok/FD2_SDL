"""
BG.DAT parser module for Fire Dragon Knight II (炎龙骑士团II)

This module implements the RLE decompression algorithm for BG.DAT battle background
images. The algorithm matches IDA function sub_4E98D (makeBgBMP) and the working
implementation in base_parser.py.

File structure:
    - Header at offset 6: 57 index entries (4 bytes each)
    - First image starts at offset 234
    - Each image has 4-byte header: [width:2][height:2] (little-endian)
    - Followed by RLE-compressed pixel data

RLE command byte format:
    b >= 192:       Skip (b & 0x3F) + 1 pixels (transparent, no src increment)
    128 <= b < 192: Fill (b & 0x3F) + 1 pixels with value from src[src_pos++]
    64 <= b < 128:  Copy (b & 0x3F) + 1 bytes from src, with extra x increment
    b < 64:         Fill (b & 0x3F) + 1 pixels with value from src[src_pos++]

The algorithm uses a flag-based state machine:
    - flag=0: Read command byte and set parameters (num7=num8=num9=0)
    - flag=1: Draw pixels using num9 (draw count) and color from current src position
"""

import struct
from typing import List, Tuple, Optional


def load_palette(fdother_path: str) -> List[Tuple[int, int, int]]:
    """Load 256-color palette from FDOTHER.DAT with 6-bit to 8-bit conversion."""
    with open(fdother_path, "rb") as f:
        fdother = f.read()

    # Parse index table to find palette location
    offsets = []
    for i in range(104):
        offset = struct.unpack("<I", fdother[6 + i * 4 : 10 + i * 4])[0]
        offsets.append(offset)

    # Palette is resource 0
    palette_start = offsets[0]
    palette_data = fdother[palette_start : palette_start + 768]

    palette = []
    for i in range(256):
        r6 = palette_data[i * 3]
        g6 = palette_data[i * 3 + 1]
        b6 = palette_data[i * 3 + 2]

        # 6-bit to 8-bit: (val << 2) | (val >> 4)
        r8 = (r6 << 2) | (r6 >> 4)
        g8 = (g6 << 2) | (g6 >> 4)
        b8 = (b6 << 2) | (b6 >> 4)

        palette.append((r8, g8, b8))

    return palette


def parse_index_table(dat_data: bytes) -> List[int]:
    """Parse BG.DAT index table starting at offset 6."""
    offsets = [234]  # First image always at offset 234

    # Parse 57 index entries (4 bytes each, starting at offset 6)
    for i in range(57):
        idx_offset = 10 + i * 4
        if idx_offset + 4 <= len(dat_data):
            offset = struct.unpack("<I", dat_data[idx_offset : idx_offset + 4])[0]
            if offset > 234 and offset < len(dat_data):
                offsets.append(offset)

    offsets.append(len(dat_data))  # End boundary
    return offsets


def decompress_bg(compressed_data: bytes, width: int, height: int) -> bytes:
    """
    Decompress BG image data using the RLE algorithm.

    Args:
        compressed_data: Raw image data including 4-byte header
        width: Image width (from header)
        height: Image height (from header)

    Returns:
        Decompressed pixel data as bytes (width * height pixel indices)
    """
    dst = bytearray(width * height)

    src_pos = 4  # Skip header
    src_end = len(compressed_data) - 1

    num7 = 0  # skip count
    num8 = 0  # cmd/repeat count
    num9 = 0  # draw count
    b = 0  # current command byte
    x = 0  # current x position
    y = 0  # current y position

    while src_pos <= src_end and y < height:
        flag = num8 != 0

        if not flag:
            # Read new command byte
            num7 = 0
            num8 = 0
            num9 = 0

            if src_pos < len(compressed_data):
                b = compressed_data[src_pos]

                if b >= 192:
                    num7 = b - 192 + 1
                if 128 <= b < 192:
                    num8 = b - 128 + 1
                if 64 <= b < 128:
                    num9 = b - 64
                    num8 = 1
                if b < 64:
                    num8 = 1
                    num9 = b

            x += num7
            if x >= width:
                x = 0
                y += 1
        else:
            # Draw mode
            count = num9

            for i in range(count + 1):
                # Extra x increment for copy mode (64-127)
                if 64 <= b < 128:
                    x += 1

                # Get color index from current src position
                if src_pos < len(compressed_data):
                    color_idx = compressed_data[src_pos]
                    if 0 <= x < width and 0 <= y < height:
                        dst[y * width + x] = color_idx

                x += 1
                if x >= width:
                    x = 0
                    y += 1
                    if y >= height:
                        break

            num8 -= 1

        src_pos += 1

    return bytes(dst)


def decode_bg_image(
    dat_data: bytes, start_offset: int, length: int, palette: List[Tuple[int, int, int]]
) -> Tuple[int, int, bytes]:
    """
    Decode a single BG image from BG.DAT.

    Args:
        dat_data: Full BG.DAT file content
        start_offset: Start offset of the image
        length: Length of the image data
        palette: 256-color palette

    Returns:
        Tuple of (width, height, rgb_data) where rgb_data is 3 bytes per pixel
    """
    if length < 4:
        raise ValueError(f"Image data too short: {length} bytes")

    # Read header
    width = struct.unpack("<h", dat_data[start_offset : start_offset + 2])[0]
    height = struct.unpack("<h", dat_data[start_offset + 2 : start_offset + 4])[0]

    if width <= 0 or height <= 0:
        raise ValueError(f"Invalid dimensions: {width}x{height}")

    # Extract compressed data (skip 4-byte header)
    compressed = dat_data[start_offset + 4 : start_offset + length]

    # Decompress to get palette indices
    pixel_indices = decompress_bg(compressed, width, height)

    # Convert to RGB using palette
    rgb_data = bytearray(width * height * 3)
    for i, idx in enumerate(pixel_indices):
        if idx < len(palette):
            r, g, b = palette[idx]
        else:
            r, g, b = idx, idx, idx
        rgb_data[i * 3] = r
        rgb_data[i * 3 + 1] = g
        rgb_data[i * 3 + 2] = b

    return width, height, bytes(rgb_data)


def save_bmp(filename: str, width: int, height: int, rgb_data: bytes):
    """Save RGB data as a BMP file."""
    # BMP requires rows to be padded to 4-byte boundary
    row_size = (width * 3 + 3) & ~3
    padding = row_size - width * 3

    file_size = 54 + row_size * height

    with open(filename, "wb") as f:
        # BMP file header (14 bytes)
        f.write(b"BM")
        f.write(struct.pack("<I", file_size))
        f.write(struct.pack("<HH", 0, 0))
        f.write(struct.pack("<I", 54))  # Pixel data offset

        # DIB header (40 bytes)
        f.write(struct.pack("<I", 40))
        f.write(struct.pack("<i", width))
        f.write(struct.pack("<i", height))
        f.write(struct.pack("<HH", 1, 24))  # Planes, bits per pixel
        f.write(struct.pack("<I", 0))  # No compression
        f.write(struct.pack("<I", row_size * height))
        f.write(struct.pack("<i", 2835))  # X pixels per meter
        f.write(struct.pack("<i", 2835))  # Y pixels per meter
        f.write(struct.pack("<I", 0))  # Colors
        f.write(struct.pack("<I", 0))  # Important colors

        # Pixel data (bottom to top, BGR format)
        padding_bytes = b"\x00" * padding
        for y in range(height - 1, -1, -1):
            row_start = y * width * 3
            row_data = rgb_data[row_start : row_start + width * 3]
            # Convert RGB to BGR
            bgr_data = bytearray()
            for i in range(0, len(row_data), 3):
                bgr_data.extend([row_data[i + 2], row_data[i + 1], row_data[i]])
            f.write(bytes(bgr_data))
            f.write(padding_bytes)


def main():
    """Main extraction function."""
    dat_path = "data/fd2/BG.DAT"
    fdother_path = "data/fd2/FDOTHER.DAT"
    output_dir = "extracted_bg"

    import os

    os.makedirs(output_dir, exist_ok=True)

    # Load data
    with open(dat_path, "rb") as f:
        dat_data = f.read()

    palette = load_palette(fdother_path)
    offsets = parse_index_table(dat_data)

    print(f"Found {len(offsets) - 1} images")

    for idx in range(len(offsets) - 1):
        start = offsets[idx]
        end = offsets[idx + 1]
        length = end - start

        if length < 4:
            continue

        try:
            width, height, rgb_data = decode_bg_image(dat_data, start, length, palette)
            output_path = os.path.join(output_dir, f"bg_{idx:03d}_{width}x{height}.bmp")
            save_bmp(output_path, width, height, rgb_data)
            print(f"  {idx}: {width}x{height} -> {output_path}")
        except Exception as e:
            print(f"  {idx}: Error - {e}")


if __name__ == "__main__":
    main()
