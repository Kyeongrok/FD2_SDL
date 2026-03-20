#!/usr/bin/env python3
"""
基于IDA Pro分析的ANI.DAT解析器
根据sub_20421函数的逻辑推断文件格式
"""

import struct
import os
import sys


def parse_ani_dat(filename):
    with open(filename, "rb") as f:
        data = f.read()

    print(f"文件大小: {len(data)} 字节")

    # 1. 检查魔数
    magic = data[0:6]
    print(f"魔数: {magic} = '{magic.decode('ascii', errors='ignore')}'")
    if magic != b"LLLLLL":
        print("警告: 魔数不匹配")

    # 2. 资源表从偏移6开始
    # 每个资源条目4字节（可能是偏移量）
    # 根据sub_20421: fseek(v5, 4 * a1 + 6, 0)
    # 所以a1是资源索引（从0开始）

    # 计算资源数量（假设直到文件末尾）
    resource_table_offset = 6
    resource_entry_size = 4
    max_resources = (len(data) - resource_table_offset) // resource_entry_size
    print(f"最大资源数: {max_resources}")

    # 读取资源表
    resources = []
    for i in range(min(max_resources, 10)):  # 只读取前10个
        entry_offset = resource_table_offset + i * resource_entry_size
        entry = struct.unpack("<I", data[entry_offset : entry_offset + 4])[0]
        resources.append(entry)
        print(f"  资源 {i}: 表条目值 0x{entry:08x} ({entry})")

    # 3. 尝试解析每个资源
    for res_idx, entry_val in enumerate(resources[:3]):  # 只尝试前3个
        print(f"\n=== 解析资源 {res_idx} ===")
        # 根据sub_20421: 第一次读取8字节到v4
        # 然后使用*(_DWORD *)v4作为偏移量
        # 所以资源表条目值可能是资源数据的偏移量？但代码中先读取8字节，然后使用第一个双字作为偏移量。
        # 实际上，代码中：
        # fseek(v5, 4 * a1 + 6, 0);  // 定位到资源表条目
        # sub_373CA(v4, 1, 8, v5);   // 读取8字节到v4
        # fseek(v5, *(_DWORD *)v4, 0); // 定位到v4中的第一个双字
        # 所以资源表条目可能不是直接的偏移量，而是指向一个8字节的结构，其中第一个双字是实际偏移量。
        # 但资源表条目是4字节，不是8字节。这里可能理解有误。

        # 让我们重新分析：资源表条目值可能是资源数据在文件中的偏移量。
        # 但代码中读取8字节，然后使用第一个双字作为偏移量。这可能意味着资源数据前有一个8字节的头。
        # 所以资源表条目值指向一个8字节的头，其中第一个双字是实际资源数据偏移量。

        # 假设资源表条目值就是资源数据偏移量
        data_offset = entry_val
        if data_offset >= len(data):
            print(f"  偏移量0x{data_offset:08x}超出文件范围")
            continue

        # 读取8字节头
        if data_offset + 8 > len(data):
            print(f"  无法读取8字节头")
            continue
        header8 = data[data_offset : data_offset + 8]
        first_dword, second_dword = struct.unpack("<II", header8)
        print(
            f"  8字节头: 第一个双字=0x{first_dword:08x}, 第二个双字=0x{second_dword:08x}"
        )

        # 使用第一个双字作为实际资源数据偏移量
        actual_offset = first_dword
        if actual_offset >= len(data):
            print(f"  实际偏移量0x{actual_offset:08x}超出文件范围")
            continue

        # 读取173字节的资源头
        # 根据代码：sub_373CA(v4, 1, 173, v5);  // 读取173字节到v4
        # 然后从v4 + 165读取一个字作为块数量
        resource_header_size = 173
        if actual_offset + resource_header_size > len(data):
            print(f"  无法读取173字节的资源头")
            continue
        resource_header = data[actual_offset : actual_offset + resource_header_size]

        # 从偏移165读取一个字（块数量）
        block_count = struct.unpack("<H", resource_header[165:167])[0]
        print(f"  资源头偏移165: 块数量 = {block_count}")

        # 打印资源头的一些信息
        print(f"  资源头前16字节: {resource_header[:16].hex()}")
        print(f"  资源头16-32字节: {resource_header[16:32].hex()}")

        # 4. 循环读取块
        # 块数据紧跟在资源头之后？代码中读取资源头后，循环读取块。
        # 循环从哪里开始？应该是资源头之后的位置。
        block_start = actual_offset + resource_header_size
        print(f"  块数据开始偏移: 0x{block_start:08x}")

        for block_idx in range(min(block_count, 5)):  # 只显示前5个块
            # 每个块先读取8字节块头
            if block_start + 8 > len(data):
                print(f"    块 {block_idx}: 无法读取块头")
                break
            block_header = data[block_start : block_start + 8]
            # 根据代码：sub_373CA(v9, 1, 8, v5);  // 读取到v9（4个__int16）
            # v9[0]是块大小，v9[1]是命令标识符？
            size, cmd, unknown1, unknown2 = struct.unpack("<HHHH", block_header)
            print(
                f"    块 {block_idx}: 大小={size}, 命令=0x{cmd:04x}, 未知1={unknown1}, 未知2={unknown2}"
            )

            # 读取块数据
            data_offset = block_start + 8
            if data_offset + size > len(data):
                print(f"      无法读取块数据")
                break
            block_data = data[data_offset : data_offset + size]
            # 显示前几个字节
            print(f"      数据前16字节: {block_data[:16].hex()}")

            # 移动到下一个块
            block_start = data_offset + size

    # 5. 尝试提取图像数据（基于我们之前的分析）
    print("\n=== 尝试提取图像数据 ===")
    # 我们知道头部有宽度和高度值
    width = struct.unpack("<H", data[0x0E:0x10])[0]
    height = struct.unpack("<H", data[0x10:0x12])[0]
    print(f"头部宽度: {width}, 高度: {height}")

    # 假设调色板在偏移0x400
    palette_offset = 0x400
    if palette_offset + 768 <= len(data):
        palette = []
        for i in range(0, 768, 3):
            r, g, b = (
                data[palette_offset + i],
                data[palette_offset + i + 1],
                data[palette_offset + i + 2],
            )
            palette.append((r, g, b))
        print(f"找到调色板在偏移0x{palette_offset:04x}")

        # 尝试提取图像数据，起始位置待定
        # 从资源分析中，我们不知道图像数据的具体位置
        # 但我们可以尝试从文件偏移0x800开始（常见位置）
        for start in [0x800, 0xC00, 0x1000, 0x1400]:
            if start + width * height <= len(data):
                image_data = data[start : start + width * height]
                # 保存为BMP
                save_bmp(
                    width,
                    height,
                    image_data,
                    palette,
                    f"extracted_{width}x{height}_0x{start:04x}.bmp",
                )
                print(f"  提取了图像 {width}x{height} 从偏移0x{start:04x}")


def save_bmp(width, height, image_data, palette, filename):
    """保存为BMP文件"""
    import struct

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
        if i < len(palette):
            r, g, b = palette[i]
        else:
            r, g, b = 0, 0, 0
        palette_data[i * 4] = b
        palette_data[i * 4 + 1] = g
        palette_data[i * 4 + 2] = r
        palette_data[i * 4 + 3] = 0

    # 写入文件
    with open(filename, "wb") as f:
        f.write(bmp_header + palette_data + image_data)


if __name__ == "__main__":
    filename = "ANI.DAT"
    if len(sys.argv) > 1:
        filename = sys.argv[1]
    if not os.path.exists(filename):
        print(f"错误: 文件 '{filename}' 不存在")
        sys.exit(1)
    parse_ani_dat(filename)
