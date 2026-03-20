#!/usr/bin/env python3
"""
FD2游戏图像提取工具
专注于从DAT文件中提取有效的图像数据
基于IDA Pro MCP服务器对渲染管线的分析
"""

import struct
import os
import sys
import json
from pathlib import Path
from typing import Dict, List, Tuple, Optional
import numpy as np


class FD2ImageExtractor:
    """FD2游戏图像提取器"""

    def __init__(self, base_path: str = "."):
        self.base_path = Path(base_path)
        self.output_dir = self.base_path / "extracted_images"
        self.output_dir.mkdir(exist_ok=True)

        # 默认调色板（256色调色板，RGB各6位）
        self.default_palette = self._create_default_palette()

        # 命令处理映射（基于IDA分析的funcs_37012数组）
        # 这些是部分已知的命令，需要进一步分析
        self.command_handlers = {
            0x00: self._cmd_fill_palette,
            0x01: self._cmd_copy_palette,
            0x02: self._cmd_process_data,
            # 更多命令将通过分析添加
        }

    def _create_default_palette(self):
        """创建默认调色板（标准256色调色板）"""
        palette = []
        for i in range(256):
            # 创建标准调色板：前16色为Windows标准色
            if i < 16:
                if i == 0:
                    r, g, b = 0, 0, 0
                elif i == 1:
                    r, g, b = 128, 0, 0
                elif i == 2:
                    r, g, b = 0, 128, 0
                elif i == 3:
                    r, g, b = 128, 128, 0
                elif i == 4:
                    r, g, b = 0, 0, 128
                elif i == 5:
                    r, g, b = 128, 0, 128
                elif i == 6:
                    r, g, b = 0, 128, 128
                elif i == 7:
                    r, g, b = 192, 192, 192
                elif i == 8:
                    r, g, b = 128, 128, 128
                elif i == 9:
                    r, g, b = 255, 0, 0
                elif i == 10:
                    r, g, b = 0, 255, 0
                elif i == 11:
                    r, g, b = 255, 255, 0
                elif i == 12:
                    r, g, b = 0, 0, 255
                elif i == 13:
                    r, g, b = 255, 0, 255
                elif i == 14:
                    r, g, b = 0, 255, 255
                else:
                    r, g, b = 255, 255, 255
            else:
                # 其他颜色使用彩虹渐变
                hue = (i - 16) * 360 / (256 - 16)
                r, g, b = self._hsv_to_rgb(hue, 1.0, 1.0)

            palette.append((r, g, b))
        return palette

    def _hsv_to_rgb(self, h, s, v):
        """HSV转RGB"""
        h = float(h)
        s = float(s)
        v = float(v)
        h60 = h / 60.0
        h60f = int(h60)
        f = h60 - h60f
        p = v * (1 - s)
        q = v * (1 - f * s)
        t = v * (1 - (1 - f) * s)

        if h60f == 0:
            r, g, b = v, t, p
        elif h60f == 1:
            r, g, b = q, v, p
        elif h60f == 2:
            r, g, b = p, v, t
        elif h60f == 3:
            r, g, b = p, q, v
        elif h60f == 4:
            r, g, b = t, p, v
        elif h60f == 5:
            r, g, b = v, p, q
        else:
            r, g, b = v, p, q

        return int(r * 255), int(g * 255), int(b * 255)

    def extract_images_from_fdother(self, filename: str = "FDOTHER.DAT") -> List[Dict]:
        """从FDOTHER.DAT提取图像"""
        filepath = self.base_path / filename
        if not filepath.exists():
            print(f"错误: 文件 {filepath} 不存在")
            return []

        print(f"=== 从{filename}提取图像 ===")

        with open(filepath, "rb") as f:
            data = f.read()

        # 1. 解析资源表
        resources = self._parse_fdother_resources(data)
        print(f"找到 {len(resources)} 个资源")

        # 2. 分析每个资源，识别图像
        images = []
        for idx, resource in enumerate(resources):
            image_data = self._try_extract_image(data, resource, idx)
            if image_data:
                images.append(image_data)
                # 保存为BMP文件
                self._save_image_as_bmp(image_data, f"fdother_res{idx:03d}")

        print(f"成功提取 {len(images)} 个图像")
        return images

    def _parse_fdother_resources(self, data: bytes) -> List[Dict]:
        """解析FDOTHER.DAT资源表"""
        resources = []
        offset = 6

        while offset + 8 <= len(data):
            start_offset, end_offset = struct.unpack("<II", data[offset : offset + 8])

            if start_offset < end_offset and end_offset <= len(data):
                size = end_offset - start_offset
                resources.append(
                    {
                        "index": len(resources),
                        "table_offset": offset,
                        "start_offset": start_offset,
                        "end_offset": end_offset,
                        "size": size,
                    }
                )

            offset += 8

            if start_offset == 0 and end_offset == 0 and len(resources) > 0:
                break

        return resources

    def _try_extract_image(
        self, data: bytes, resource: Dict, index: int
    ) -> Optional[Dict]:
        """尝试将资源数据解释为图像"""
        start = resource["start_offset"]
        end = resource["end_offset"]
        size = resource["size"]

        # 常见图像尺寸
        common_sizes = [
            (320, 200),
            (640, 480),
            (800, 600),  # 标准VGA/SVGA
            (398, 25),
            (32, 321),
            (256, 256),  # 从头部信息发现的尺寸
            (128, 128),
            (64, 64),
            (32, 32),  # 其他可能尺寸
        ]

        for width, height in common_sizes:
            if width * height == size:
                # 尝试作为8位调色板索引图像
                image_data = data[start:end]

                # 检查数据是否有足够的变化（不是全零或全相同）
                unique_bytes = len(set(image_data))
                if unique_bytes > 10:  # 至少有10种不同的颜色
                    return {
                        "index": index,
                        "width": width,
                        "height": height,
                        "size": size,
                        "start_offset": start,
                        "end_offset": end,
                        "data": image_data,
                        "unique_bytes": unique_bytes,
                        "palette": self.default_palette,
                    }

        return None

    def _save_image_as_bmp(self, image_data: Dict, base_name: str):
        """将图像数据保存为BMP文件"""
        width = image_data["width"]
        height = image_data["height"]
        image_bytes = image_data["data"]
        palette = image_data["palette"]

        # 创建BMP文件
        bmp_header_size = 14 + 40
        bmp_header = bytearray(bmp_header_size)

        # BMP文件头
        bmp_header[0:2] = b"BM"
        file_size = bmp_header_size + 1024 + len(image_bytes)
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
            f.write(bmp_header + palette_data + image_bytes)

        print(f"  图像已保存: {output_file} ({width}x{height})")

    def extract_images_from_ani(self, filename: str = "ANI.DAT") -> List[Dict]:
        """从ANI.DAT提取图像（通过解码命令）"""
        filepath = self.base_path / filename
        if not filepath.exists():
            print(f"错误: 文件 {filepath} 不存在")
            return []

        print(f"\n=== 从{filename}提取图像（命令解码）===")

        with open(filepath, "rb") as f:
            data = f.read()

        # 1. 解析资源表
        resources = self._parse_ani_resources(data)
        print(f"找到 {len(resources)} 个资源")

        # 2. 解码每个资源
        images = []
        for idx, resource in enumerate(resources[:5]):  # 先处理前5个
            try:
                resource_images = self._decode_ani_resource(data, resource, idx)
                images.extend(resource_images)
            except Exception as e:
                print(f"  解码资源 {idx} 失败: {e}")

        print(f"从ANI.DAT解码出 {len(images)} 个图像")
        return images

    def _parse_ani_resources(self, data: bytes) -> List[Dict]:
        """解析ANI.DAT资源表"""
        resources = []
        offset = 6

        while offset + 8 <= len(data):
            start_offset, end_offset = struct.unpack("<II", data[offset : offset + 8])

            if start_offset < end_offset and end_offset <= len(data):
                resources.append(
                    {
                        "index": len(resources),
                        "table_offset": offset,
                        "start_offset": start_offset,
                        "end_offset": end_offset,
                        "size": end_offset - start_offset,
                    }
                )

            offset += 8

            if start_offset == 0 and end_offset == 0 and len(resources) > 0:
                break

        return resources

    def _decode_ani_resource(
        self, data: bytes, resource: Dict, index: int
    ) -> List[Dict]:
        """解码ANI.DAT资源（通过命令系统）"""
        start = resource["start_offset"]
        end = resource["end_offset"]

        # 读取8字节头
        if start + 8 > len(data):
            return []

        header8 = data[start : start + 8]
        first_dword, second_dword = struct.unpack("<II", header8)

        # 使用第一个双字作为实际偏移量（根据IDA分析）
        actual_offset = first_dword
        if actual_offset >= len(data):
            actual_offset = start

        # 读取173字节资源头
        if actual_offset + 173 > len(data):
            return []

        resource_header = data[actual_offset : actual_offset + 173]

        # 从偏移165读取块数量
        if 165 + 2 > len(resource_header):
            block_count = 0
        else:
            block_count = struct.unpack("<H", resource_header[165:167])[0]

        # 解码块数据
        images = []
        current_image = None
        palette = self.default_palette.copy()
        block_start = actual_offset + 173

        for block_idx in range(min(block_count, 100)):
            if block_start + 8 > len(data):
                break

            # 读取块头
            block_header = data[block_start : block_start + 8]
            size, cmd, unknown1, unknown2 = struct.unpack("<HHHH", block_header)

            # 读取块数据
            data_start = block_start + 8
            if data_start + size > len(data):
                break

            block_data = data[data_start : data_start + size]

            # 处理命令
            if cmd in self.command_handlers:
                result = self.command_handlers[cmd](block_data, palette, current_image)
                if result:
                    if result.get("type") == "image":
                        current_image = result
                        images.append(current_image)
                    elif result.get("type") == "palette":
                        palette = result.get("palette", palette)

            block_start = data_start + size

        return images

    # 命令处理函数（基于IDA分析）
    def _cmd_fill_palette(
        self, data: bytes, palette: List, current_image: Optional[Dict]
    ) -> Optional[Dict]:
        """命令0：填充调色板区域"""
        # 根据IDA分析，此命令读取一个字节并重复填充
        if len(data) < 1:
            return None

        value = data[0]
        # 填充调色板的前192个条目
        for i in range(192):
            if i < len(palette):
                r, g, b = palette[i]
                # 根据value调整颜色
                new_r = min(255, r + value)
                new_g = min(255, g + value)
                new_b = min(255, b + value)
                palette[i] = (new_r, new_g, new_b)

        return {"type": "palette", "palette": palette}

    def _cmd_copy_palette(
        self, data: bytes, palette: List, current_image: Optional[Dict]
    ) -> Optional[Dict]:
        """命令1：复制调色板块"""
        # 根据IDA分析，此命令复制内存块
        # 这里简单地将数据解释为调色板
        if len(data) >= 768:  # 256个RGB颜色
            new_palette = []
            for i in range(0, 768, 3):
                if i + 2 < len(data):
                    r, g, b = data[i], data[i + 1], data[i + 2]
                    new_palette.append((r, g, b))

            if len(new_palette) == 256:
                return {"type": "palette", "palette": new_palette}

        return None

    def _cmd_process_data(
        self, data: bytes, palette: List, current_image: Optional[Dict]
    ) -> Optional[Dict]:
        """命令2：处理数据（可能是图像数据）"""
        # 根据之前分析，命令2处理数据
        # 我们尝试将数据解释为图像

        # 尝试常见图像尺寸
        for width, height in [(320, 200), (398, 25), (32, 321), (256, 256)]:
            if width * height == len(data):
                # 创建图像
                return {
                    "type": "image",
                    "width": width,
                    "height": height,
                    "data": data,
                    "palette": palette.copy(),
                }

        return None

    def analyze_palette_sources(self):
        """分析文件中的调色板来源"""
        print("\n=== 分析调色板来源 ===")

        # 检查FDOTHER.DAT中的调色板
        fdother_path = self.base_path / "FDOTHER.DAT"
        if fdother_path.exists():
            with open(fdother_path, "rb") as f:
                data = f.read()

            # 查找可能的调色板数据（256个RGB三元组）
            for offset in range(0, min(0x10000, len(data) - 768), 4):
                palette_data = data[offset : offset + 768]
                # 检查是否是有效的调色板
                valid = True
                for i in range(0, 768, 3):
                    r, g, b = palette_data[i], palette_data[i + 1], palette_data[i + 2]
                    if r > 63 or g > 63 or b > 63:  # 可能是6位颜色
                        if r > 255 or g > 255 or b > 255:
                            valid = False
                            break

                if valid:
                    print(f"  FDOTHER.DAT 偏移 0x{offset:04x}: 可能的调色板")
                    # 保存调色板
                    self._save_palette(palette_data, f"fdother_palette_0x{offset:04x}")

        print("调色板分析完成")

    def _save_palette(self, palette_data: bytes, name: str):
        """保存调色板为文件"""
        output_file = self.output_dir / f"{name}.pal"
        with open(output_file, "wb") as f:
            f.write(palette_data)

    def generate_comprehensive_report(self):
        """生成综合分析报告"""
        print("\n=== 生成综合分析报告 ===")

        # 提取FDOTHER.DAT图像
        fdother_images = self.extract_images_from_fdother()

        # 提取ANI.DAT图像
        ani_images = self.extract_images_from_ani()

        # 分析调色板来源
        self.analyze_palette_sources()

        # 生成报告
        report = {
            "fdother_images_count": len(fdother_images),
            "ani_images_count": len(ani_images),
            "total_images": len(fdother_images) + len(ani_images),
            "fdother_images": [
                {
                    "index": img["index"],
                    "width": img["width"],
                    "height": img["height"],
                    "size": img["size"],
                    "start_offset": img["start_offset"],
                }
                for img in fdother_images
            ],
            "ani_images": [
                {
                    "width": img.get("width", 0),
                    "height": img.get("height", 0),
                    "size": len(img.get("data", b"")),
                }
                for img in ani_images
            ],
        }

        report_file = self.output_dir / "image_extraction_report.json"
        with open(report_file, "w") as f:
            json.dump(report, f, indent=2)

        print(f"综合报告已保存到: {report_file}")
        print(f"总共提取了 {report['total_images']} 个图像")

        return report


def main():
    """主函数"""
    if len(sys.argv) > 1:
        base_path = sys.argv[1]
    else:
        base_path = "."

    extractor = FD2ImageExtractor(base_path)

    print("FD2游戏图像提取工具")
    print("=" * 60)
    print("专注于提取有效的图像数据")
    print("=" * 60)

    # 生成综合报告
    report = extractor.generate_comprehensive_report()

    print("\n" + "=" * 60)
    print("图像提取完成！")
    print(f"图像保存在: {extractor.output_dir}")
    print(f"共提取了 {report['total_images']} 个图像")
    print("\n下一步：")
    print("1. 查看提取的BMP图像文件")
    print("2. 使用图形编辑器检查图像质量")
    print("3. 如果需要，调整调色板或解码参数")


if __name__ == "__main__":
    main()
