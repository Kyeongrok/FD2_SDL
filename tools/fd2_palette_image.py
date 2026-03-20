#!/usr/bin/env python3
"""
FD2调色板和图像提取工具
尝试从FDOTHER.DAT中提取调色板和图像
"""

import struct
import os
import sys


def find_palette_in_fdother():
    """在FDOTHER.DAT中查找调色板资源"""
    with open("FDOTHER.DAT", "rb") as f:
        data = f.read()

    # 解析资源表
    resources = []
    offset = 6
    while offset + 8 <= len(data):
        start, end = struct.unpack("<II", data[offset : offset + 8])
        if start < end and end <= len(data):
            size = end - start
            resources.append((start, end, size))
        offset += 8
        if start == 0 and end == 0 and len(resources) > 0:
            break

    print(f"找到 {len(resources)} 个资源")

    # 查找大小为768的资源（256个RGB颜色）
    palette_candidates = []
    for idx, (start, end, size) in enumerate(resources):
        if size == 768:
            palette_data = data[start:end]
            # 检查是否是有效的调色板
            valid = True
            for i in range(0, 768, 3):
                r, g, b = palette_data[i], palette_data[i + 1], palette_data[i + 2]
                if r > 63 or g > 63 or b > 63:  # 可能是6位颜色
                    if r > 255 or g > 255 or b > 255:
                        valid = False
                        break

            if valid:
                palette_candidates.append((idx, start, palette_data))
                print(f"  资源 {idx}: 偏移 0x{start:08x}, 大小768, 可能是调色板")

    # 尝试使用每个调色板候选提取图像
    for pal_idx, (res_idx, pal_start, palette_data) in enumerate(
        palette_candidates[:3]
    ):
        print(f"\n尝试使用调色板候选 {pal_idx} (资源 {res_idx})")
        extract_images_with_palette(data, resources, palette_data, f"palette{pal_idx}")


def extract_images_with_palette(data, resources, palette, palette_name):
    """使用指定调色板提取图像"""
    # 常见图像尺寸
    common_sizes = [
        (320, 200),
        (640, 480),
        (800, 600),
        (398, 25),
        (32, 321),
        (256, 256),
        (128, 128),
        (64, 64),
        (32, 32),
        (160, 100),
        (160, 200),
    ]

    extracted = 0
    for idx, (start, end, size) in enumerate(resources[:50]):  # 检查前50个资源
        for width, height in common_sizes:
            if width * height == size:
                # 提取图像数据
                image_data = data[start:end]

                # 检查数据是否有足够的变化
                unique_bytes = len(set(image_data))
                if unique_bytes > 10:
                    # 保存为BMP
                    save_bmp(
                        width,
                        height,
                        image_data,
                        palette,
                        f"{palette_name}_res{idx}_{width}x{height}.bmp",
                    )
                    print(f"    资源 {idx}: {width}x{height}, 唯一字节: {unique_bytes}")
                    extracted += 1
                    break

    print(f"  使用此调色板提取了 {extracted} 个图像")


def save_bmp(width, height, image_data, palette, filename):
    """保存为BMP文件"""
    bmp_header_size = 14 + 40
    bmp_header = bytearray(bmp_header_size)

    # BMP文件头
    bmp_header[0:2] = b"BM"
    file_size = bmp_header_size + 1024 + len(image_data)
    bmp_header[2:6] = struct.pack("<I", file_size)
    bmp_header[6:10] = struct.pack("<I", 0)
    bmp_header[10:14] = struct.pack("<I", bmp_header_size + 1024)

    # BMP信息头
    bmp_header[14:18] = struct.pack("<I", 40)
    bmp_header[18:22] = struct.pack("<i", width)
    bmp_header[22:26] = struct.pack("<i", height)
    bmp_header[26:28] = struct.pack("<H", 1)
    bmp_header[28:30] = struct.pack("<H", 8)
    bmp_header[30:34] = struct.pack("<I", 0)
    bmp_header[34:38] = struct.pack("<I", width * height)
    bmp_header[38:42] = struct.pack("<I", 2835)
    bmp_header[42:46] = struct.pack("<I", 2835)
    bmp_header[46:50] = struct.pack("<I", 256)
    bmp_header[50:54] = struct.pack("<I", 0)

    # 调色板数据
    palette_data = bytearray(1024)
    for i in range(256):
        if i * 3 + 2 < len(palette):
            r, g, b = palette[i * 3], palette[i * 3 + 1], palette[i * 3 + 2]
        else:
            r, g, b = 0, 0, 0
        palette_data[i * 4] = b
        palette_data[i * 4 + 1] = g
        palette_data[i * 4 + 2] = r
        palette_data[i * 4 + 3] = 0

    # 保存文件
    output_dir = "extracted_images"
    os.makedirs(output_dir, exist_ok=True)
    output_file = os.path.join(output_dir, filename)
    with open(output_file, "wb") as f:
        f.write(bmp_header + palette_data + image_data)


def try_direct_image_extraction():
    """尝试直接提取图像（不使用调色板）"""
    print("\n=== 尝试直接提取图像 ===")

    with open("FDOTHER.DAT", "rb") as f:
        data = f.read()

    # 解析资源表
    resources = []
    offset = 6
    while offset + 8 <= len(data):
        start, end = struct.unpack("<II", data[offset : offset + 8])
        if start < end and end <= len(data):
            size = end - start
            resources.append((start, end, size))
        offset += 8
        if start == 0 and end == 0 and len(resources) > 0:
            break

    # 尝试常见尺寸，使用灰度调色板
    common_sizes = [
        (320, 200),
        (640, 480),
        (800, 600),
        (398, 25),
        (32, 321),
        (256, 256),
        (128, 128),
        (64, 64),
        (32, 32),
    ]

    extracted = 0
    for idx, (start, end, size) in enumerate(resources[:50]):
        for width, height in common_sizes:
            if width * height == size:
                image_data = data[start:end]
                unique_bytes = len(set(image_data))
                if unique_bytes > 10:
                    # 使用灰度调色板
                    palette = []
                    for i in range(256):
                        g = i
                        palette.extend([g, g, g])

                    save_bmp(
                        width,
                        height,
                        image_data,
                        palette,
                        f"direct_res{idx}_{width}x{height}.bmp",
                    )
                    print(f"  资源 {idx}: {width}x{height}, 唯一字节: {unique_bytes}")
                    extracted += 1
                    break

    print(f"直接提取了 {extracted} 个图像")


def analyze_data_patterns():
    """分析数据模式，寻找图像特征"""
    print("\n=== 分析数据模式 ===")

    with open("FDOTHER.DAT", "rb") as f:
        data = f.read()

    # 查找可能的图像数据模式
    # 1. 查找重复的字节模式（可能是图像数据）
    # 2. 查找零填充区域（可能是背景）
    # 3. 查找渐变区域

    # 简单分析：检查每个资源的数据分布
    resources = []
    offset = 6
    while offset + 8 <= len(data):
        start, end = struct.unpack("<II", data[offset : offset + 8])
        if start < end and end <= len(data):
            size = end - start
            resources.append((start, end, size))
        offset += 8
        if start == 0 and end == 0 and len(resources) > 0:
            break

    print(f"分析 {len(resources)} 个资源的数据模式:")

    for idx, (start, end, size) in enumerate(resources[:10]):
        resource_data = data[start:end]

        # 计算统计信息
        unique_bytes = len(set(resource_data))
        zero_count = resource_data.count(0)

        # 查找常见字节值
        byte_freq = {}
        for byte in resource_data:
            byte_freq[byte] = byte_freq.get(byte, 0) + 1
        common_bytes = sorted(byte_freq.items(), key=lambda x: x[1], reverse=True)[:5]

        print(f"  资源 {idx} (大小{size}): 唯一字节{unique_bytes}, 零字节{zero_count}")
        if common_bytes:
            print(f"    常见字节: {common_bytes}")

        # 检查是否可能是图像数据
        if size in [w * h for w, h in [(320, 200), (398, 25), (32, 321)]]:
            print(f"    可能是图像尺寸: {size} = 320x200 或 398x25 或 32x321")


if __name__ == "__main__":
    print("FD2调色板和图像提取工具")
    print("=" * 60)

    # 分析数据模式
    analyze_data_patterns()

    # 尝试直接提取图像
    try_direct_image_extraction()

    # 查找调色板并提取图像
    find_palette_in_fdother()

    print("\n" + "=" * 60)
    print("提取完成！检查 extracted_images 目录")
