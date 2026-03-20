#!/usr/bin/env python3
"""
FD2高级图像提取工具
实施多种调色板格式检测和图像解码方法
基于IDA Pro MCP服务器对渲染管线的分析
"""

import struct
import os
import sys
import json
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import itertools


class FD2AdvancedExtractor:
    """FD2高级图像提取器"""

    def __init__(self, base_path: str = "."):
        self.base_path = Path(base_path)
        self.output_dir = self.base_path / "advanced_extraction"
        self.output_dir.mkdir(exist_ok=True)

        # 调色板格式检测方法
        self.palette_formats = {
            "6bit_RGB": self._decode_6bit_palette,
            "6bit_BGR": self._decode_6bit_palette_bgr,
            "8bit_RGB": self._decode_8bit_palette,
            "8bit_BGR": self._decode_8bit_palette_bgr,
            "6bit_RGB_shift": self._decode_6bit_palette_shift,
            "6bit_BGR_shift": self._decode_6bit_palette_bgr_shift,
        }

        # 图像解码方法
        self.image_decoders = {
            "raw": self._decode_raw_image,
            "rle": self._decode_rle_image,
            "planar": self._decode_planar_image,
            "xor": self._decode_xor_image,
        }

        # 常见图像尺寸
        self.common_sizes = [
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
            (320, 240),
        ]

    # 调色板解码方法
    def _decode_6bit_palette(self, data: bytes) -> List[Tuple[int, int, int]]:
        """6位RGB调色板（0-63范围）"""
        palette = []
        for i in range(0, min(768, len(data)), 3):
            if i + 2 >= len(data):
                break
            r6, g6, b6 = data[i] & 0x3F, data[i + 1] & 0x3F, data[i + 2] & 0x3F
            # 6位扩展到8位（重复高2位）
            r = (r6 << 2) | (r6 >> 4)
            g = (g6 << 2) | (g6 >> 4)
            b = (b6 << 2) | (b6 >> 4)
            palette.append((r, g, b))
        return palette

    def _decode_6bit_palette_bgr(self, data: bytes) -> List[Tuple[int, int, int]]:
        """6位BGR调色板"""
        palette = []
        for i in range(0, min(768, len(data)), 3):
            if i + 2 >= len(data):
                break
            b6, g6, r6 = data[i] & 0x3F, data[i + 1] & 0x3F, data[i + 2] & 0x3F
            r = (r6 << 2) | (r6 >> 4)
            g = (g6 << 2) | (g6 >> 4)
            b = (b6 << 2) | (b6 >> 4)
            palette.append((r, g, b))
        return palette

    def _decode_8bit_palette(self, data: bytes) -> List[Tuple[int, int, int]]:
        """8位RGB调色板"""
        palette = []
        for i in range(0, min(768, len(data)), 3):
            if i + 2 >= len(data):
                break
            r, g, b = data[i], data[i + 1], data[i + 2]
            palette.append((r, g, b))
        return palette

    def _decode_8bit_palette_bgr(self, data: bytes) -> List[Tuple[int, int, int]]:
        """8位BGR调色板"""
        palette = []
        for i in range(0, min(768, len(data)), 3):
            if i + 2 >= len(data):
                break
            b, g, r = data[i], data[i + 1], data[i + 2]
            palette.append((r, g, b))
        return palette

    def _decode_6bit_palette_shift(self, data: bytes) -> List[Tuple[int, int, int]]:
        """6位RGB调色板（带移位）"""
        palette = []
        for i in range(0, min(768, len(data)), 3):
            if i + 2 >= len(data):
                break
            r6, g6, b6 = data[i] >> 2, data[i + 1] >> 2, data[i + 2] >> 2
            r = r6 << 2
            g = g6 << 2
            b = b6 << 2
            palette.append((r, g, b))
        return palette

    def _decode_6bit_palette_bgr_shift(self, data: bytes) -> List[Tuple[int, int, int]]:
        """6位BGR调色板（带移位）"""
        palette = []
        for i in range(0, min(768, len(data)), 3):
            if i + 2 >= len(data):
                break
            b6, g6, r6 = data[i] >> 2, data[i + 1] >> 2, data[i + 2] >> 2
            r = r6 << 2
            g = g6 << 2
            b = b6 << 2
            palette.append((r, g, b))
        return palette

    # 图像解码方法
    def _decode_raw_image(
        self, data: bytes, width: int, height: int
    ) -> Optional[bytes]:
        """原始图像数据"""
        if width * height <= len(data):
            return data[: width * height]
        return None

    def _decode_rle_image(
        self, data: bytes, width: int, height: int
    ) -> Optional[bytes]:
        """RLE解码"""
        decoded = bytearray()
        i = 0
        while i < len(data) and len(decoded) < width * height:
            if data[i] == 0x00 and i + 1 < len(data):
                # RLE标记
                count = data[i + 1]
                if count > 0 and i + 2 < len(data):
                    value = data[i + 2]
                    for _ in range(count):
                        decoded.append(value)
                    i += 3
                else:
                    i += 2
            else:
                decoded.append(data[i])
                i += 1

        if len(decoded) >= width * height:
            return bytes(decoded[: width * height])
        return None

    def _decode_planar_image(
        self, data: bytes, width: int, height: int
    ) -> Optional[bytes]:
        """位平面解码（VGA 4位平面格式）"""
        if width * height // 8 * 4 > len(data):
            return None

        decoded = bytearray()
        plane_size = width * height // 8

        for y in range(height):
            row = bytearray()
            for x in range(0, width, 8):
                # 读取4个平面的字节
                plane0 = (
                    data[0 * plane_size + y * (width // 8) + x // 8]
                    if 0 * plane_size + y * (width // 8) + x // 8 < len(data)
                    else 0
                )
                plane1 = (
                    data[1 * plane_size + y * (width // 8) + x // 8]
                    if 1 * plane_size + y * (width // 8) + x // 8 < len(data)
                    else 0
                )
                plane2 = (
                    data[2 * plane_size + y * (width // 8) + x // 8]
                    if 2 * plane_size + y * (width // 8) + x // 8 < len(data)
                    else 0
                )
                plane3 = (
                    data[3 * plane_size + y * (width // 8) + x // 8]
                    if 3 * plane_size + y * (width // 8) + x // 8 < len(data)
                    else 0
                )

                # 解包8个像素
                for bit in range(8):
                    pixel = ((plane3 >> (7 - bit)) & 1) << 3
                    pixel |= ((plane2 >> (7 - bit)) & 1) << 2
                    pixel |= ((plane1 >> (7 - bit)) & 1) << 1
                    pixel |= (plane0 >> (7 - bit)) & 1
                    decoded.append(pixel)

        if len(decoded) >= width * height:
            return bytes(decoded[: width * height])
        return None

    def _decode_xor_image(
        self, data: bytes, width: int, height: int
    ) -> Optional[bytes]:
        """XOR解码"""
        if width * height <= len(data):
            decoded = bytearray()
            prev = 0
            for byte in data[: width * height]:
                decoded.append(byte ^ prev)
                prev = byte
            return bytes(decoded)
        return None

    def find_palettes(
        self, data: bytes
    ) -> List[Tuple[int, str, List[Tuple[int, int, int]]]]:
        """查找所有可能的调色板"""
        palettes = []

        # 解析资源表
        resources = self._parse_resources(data)

        for idx, resource in enumerate(resources):
            start, end, size = resource
            if size == 768:
                palette_data = data[start:end]

                # 尝试所有调色板格式
                for format_name, decode_func in self.palette_formats.items():
                    try:
                        palette = decode_func(palette_data)
                        if len(palette) == 256:
                            palettes.append((start, format_name, palette))
                    except:
                        continue

        return palettes

    def _parse_resources(self, data: bytes) -> List[Tuple[int, int, int]]:
        """解析资源表"""
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

        return resources

    def extract_images_with_all_palettes(self, filename: str = "FDOTHER.DAT"):
        """使用所有可能的调色板提取图像"""
        filepath = self.base_path / filename
        if not filepath.exists():
            print(f"错误: 文件 {filepath} 不存在")
            return

        print(f"=== 使用所有调色板格式提取图像: {filename} ===")

        with open(filepath, "rb") as f:
            data = f.read()

        # 查找所有调色板
        palettes = self.find_palettes(data)
        print(f"找到 {len(palettes)} 个可能的调色板")

        # 解析资源
        resources = self._parse_resources(data)
        print(f"找到 {len(resources)} 个资源")

        # 尝试使用每个调色板提取图像
        all_images = []
        for pal_idx, (pal_offset, format_name, palette) in enumerate(
            palettes[:5]
        ):  # 限制调色板数量
            print(
                f"\n使用调色板 #{pal_idx} (偏移 0x{pal_offset:08x}, 格式: {format_name}):"
            )

            images = self._extract_images_with_palette(
                data, resources, palette, pal_offset, format_name
            )
            all_images.extend(images)

            if images:
                print(f"  提取了 {len(images)} 个图像")
                # 保存前几个图像
                for img_idx, img in enumerate(images[:3]):
                    self._save_image(img, f"pal{pal_idx}_{format_name}_{img_idx}")
            else:
                print(f"  未提取到图像")

        # 生成报告
        self._generate_report(all_images, palettes, resources)

        print(f"\n总共提取了 {len(all_images)} 个图像")
        return all_images

    def _extract_images_with_palette(
        self,
        data: bytes,
        resources: List[Tuple[int, int, int]],
        palette: List[Tuple[int, int, int]],
        palette_offset: int,
        format_name: str,
    ) -> List[Dict]:
        """使用指定调色板提取图像"""
        images = []

        for res_idx, (start, end, size) in enumerate(resources):
            # 尝试常见尺寸
            for width, height in self.common_sizes:
                if width * height == size:
                    image_data = data[start:end]

                    # 尝试所有图像解码方法
                    for decoder_name, decode_func in self.image_decoders.items():
                        decoded_data = decode_func(image_data, width, height)
                        if decoded_data:
                            # 检查图像质量
                            if self._evaluate_image_quality(decoded_data, palette):
                                images.append(
                                    {
                                        "resource_index": res_idx,
                                        "start_offset": start,
                                        "end_offset": end,
                                        "width": width,
                                        "height": height,
                                        "size": size,
                                        "palette_offset": palette_offset,
                                        "palette_format": format_name,
                                        "decoder": decoder_name,
                                        "data": decoded_data,
                                        "palette": palette,
                                    }
                                )

        return images

    def _evaluate_image_quality(
        self, image_data: bytes, palette: List[Tuple[int, int, int]]
    ) -> bool:
        """评估图像质量"""
        # 检查是否有足够的颜色变化
        unique_indices = len(set(image_data))
        if unique_indices < 10:
            return False

        # 检查是否有大量黑色或白色（可能是无效数据）
        black_count = image_data.count(0)
        white_count = image_data.count(255)
        if black_count > len(image_data) * 0.9 or white_count > len(image_data) * 0.9:
            return False

        return True

    def _save_image(self, image: Dict, base_name: str):
        """保存图像为BMP文件"""
        width = image["width"]
        height = image["height"]
        image_data = image["data"]
        palette = image["palette"]

        # 创建BMP文件
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

        # 保存文件
        output_file = self.output_dir / f"{base_name}.bmp"
        with open(output_file, "wb") as f:
            f.write(bmp_header + palette_data + image_data)

    def _generate_report(
        self, images: List[Dict], palettes: List, resources: List[Tuple[int, int, int]]
    ):
        """生成分析报告"""
        report = {
            "total_palettes_found": len(palettes),
            "total_resources": len(resources),
            "total_images_extracted": len(images),
            "palettes": [
                {
                    "offset": offset,
                    "format": format_name,
                    "sample_colors": palette[:10],  # 前10个颜色的样本
                }
                for offset, format_name, palette in palettes[:5]
            ],
            "images": [
                {
                    "resource_index": img["resource_index"],
                    "width": img["width"],
                    "height": img["height"],
                    "size": img["size"],
                    "palette_format": img["palette_format"],
                    "decoder": img["decoder"],
                }
                for img in images
            ],
        }

        report_file = self.output_dir / "advanced_extraction_report.json"
        with open(report_file, "w") as f:
            json.dump(report, f, indent=2)

        print(f"分析报告已保存到: {report_file}")


def main():
    """主函数"""
    if len(sys.argv) > 1:
        base_path = sys.argv[1]
    else:
        base_path = "."

    extractor = FD2AdvancedExtractor(base_path)

    print("FD2高级图像提取工具")
    print("=" * 60)
    print("实施多种调色板格式检测")
    print("=" * 60)

    # 使用所有调色板格式提取图像
    images = extractor.extract_images_with_all_palettes()

    print("\n" + "=" * 60)
    print("高级提取完成！")
    print(f"图像保存在: {extractor.output_dir}")
    print(f"共提取了 {len(images)} 个图像")
    print("\n下一步：")
    print("1. 检查生成的BMP图像，哪些看起来正确？")
    print("2. 记录有效的调色板格式和解码方法")
    print("3. 使用这些参数改进提取工具")


if __name__ == "__main__":
    main()
