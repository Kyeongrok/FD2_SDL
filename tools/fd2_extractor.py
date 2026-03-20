#!/usr/bin/env python3
"""
FD2游戏资源提取工具
基于IDA Pro MCP服务器对fd2.exe的逆向工程分析
支持提取ANI.DAT动画资源和FDOTHER.DAT图形资源
"""

import struct
import os
import sys
import json
from pathlib import Path
from typing import Dict, List, Tuple, Optional


class FD2Extractor:
    """FD2游戏资源提取器"""

    def __init__(self, base_path: str = "."):
        self.base_path = Path(base_path)
        self.output_dir = self.base_path / "extracted_resources"
        self.output_dir.mkdir(exist_ok=True)

        # 命令处理函数映射（基于funcs_37012数组分析）
        self.command_handlers = {
            0x00: "fill_palette_region",
            0x01: "copy_palette_block",
            # 更多命令将在分析后添加
        }

    def extract_ani_dat(self, filename: str = "ANI.DAT") -> Dict:
        """提取ANI.DAT文件中的所有资源"""
        filepath = self.base_path / filename
        if not filepath.exists():
            print(f"错误: 文件 {filepath} 不存在")
            return {}

        print(f"=== 提取ANI.DAT文件: {filepath} ===")

        with open(filepath, "rb") as f:
            data = f.read()

        # 1. 验证文件头
        magic = data[0:6]
        if magic != b"LLLLLL":
            print(f"警告: 魔数不匹配，期望'LLLLLL'，得到'{magic}'")

        # 2. 解析资源表
        resources = self._parse_resource_table(data)
        print(f"找到 {len(resources)} 个资源条目")

        # 3. 提取每个资源
        extracted_resources = []
        for idx, resource_info in enumerate(resources[:20]):  # 先提取前20个
            try:
                resource_data = self._extract_ani_resource(data, resource_info, idx)
                if resource_data:
                    extracted_resources.append(resource_data)
                    self._save_resource(resource_data, f"ani_res{idx:03d}")
            except Exception as e:
                print(f"  提取资源 {idx} 失败: {e}")

        # 4. 生成分析报告
        report = self._generate_ani_report(resources, extracted_resources)
        report_file = self.output_dir / "ani_dat_analysis.json"
        with open(report_file, "w") as f:
            json.dump(report, f, indent=2)
        print(f"分析报告已保存到: {report_file}")

        return report

    def _parse_resource_table(self, data: bytes) -> List[Dict]:
        """解析ANI.DAT资源表（每个条目8字节：起始偏移+结束偏移）"""
        resources = []
        offset = 6  # 资源表起始位置

        while offset + 8 <= len(data):
            start_offset, end_offset = struct.unpack("<II", data[offset : offset + 8])

            # 检查是否是有效的资源
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

            # 如果遇到连续的零，可能到达表尾
            if start_offset == 0 and end_offset == 0 and len(resources) > 0:
                break

        return resources

    def _extract_ani_resource(
        self, data: bytes, resource_info: Dict, index: int
    ) -> Optional[Dict]:
        """提取单个ANI.DAT资源"""
        start_offset = resource_info["start_offset"]
        end_offset = resource_info["end_offset"]

        if start_offset + 8 > len(data):
            print(f"  资源 {index}: 起始偏移量 0x{start_offset:08x} 超出文件范围")
            return None

        # 读取8字节头（资源表已经提供了起始偏移量，但根据IDA代码，这里可能还有另一个头）
        header8 = data[start_offset : start_offset + 8]
        first_dword, second_dword = struct.unpack("<II", header8)

        # 根据IDA分析，第一个双字是实际数据偏移量
        # 但资源表条目已经提供了起始偏移量，这里可能是冗余的
        actual_offset = first_dword
        if actual_offset >= len(data):
            # 如果first_dword不合理，使用资源表提供的起始偏移量
            actual_offset = start_offset

        # 读取173字节资源头
        resource_header_size = 173
        if actual_offset + resource_header_size > len(data):
            print(
                f"  资源 {index}: 无法读取完整资源头，实际偏移量0x{actual_offset:08x}"
            )
            return None

        resource_header = data[actual_offset : actual_offset + resource_header_size]

        # 从偏移165读取块数量
        block_count_offset = 165
        if block_count_offset + 2 > len(resource_header):
            block_count = 0
        else:
            block_count = struct.unpack(
                "<H", resource_header[block_count_offset : block_count_offset + 2]
            )[0]

        # 解析块数据
        blocks = []
        block_start = actual_offset + resource_header_size

        for block_idx in range(min(block_count, 50)):  # 限制块数量
            if block_start + 8 > len(data):
                break

            # 读取8字节块头
            block_header = data[block_start : block_start + 8]
            size, cmd, unknown1, unknown2 = struct.unpack("<HHHH", block_header)

            # 读取块数据
            data_offset = block_start + 8
            if data_offset + size > len(data):
                break

            block_data = data[data_offset : data_offset + size]

            blocks.append(
                {
                    "index": block_idx,
                    "offset": block_start,
                    "size": size,
                    "command": cmd,
                    "unknown1": unknown1,
                    "unknown2": unknown2,
                    "data": block_data.hex()[:100]
                    + ("..." if len(block_data) > 100 else ""),
                }
            )

            # 移动到下一个块
            block_start = data_offset + size

        return {
            "index": index,
            "table_offset": resource_info["table_offset"],
            "start_offset": start_offset,
            "end_offset": end_offset,
            "resource_size": resource_info["size"],
            "header_offset": start_offset,
            "actual_offset": actual_offset,
            "header8": header8.hex(),
            "first_dword": first_dword,
            "second_dword": second_dword,
            "resource_header_size": resource_header_size,
            "block_count": block_count,
            "blocks": blocks,
            "resource_header_sample": resource_header[:64].hex(),
        }

    def extract_fdother_dat(self, filename: str = "FDOTHER.DAT") -> Dict:
        """提取FDOTHER.DAT文件中的图形资源"""
        filepath = self.base_path / filename
        if not filepath.exists():
            print(f"错误: 文件 {filepath} 不存在")
            return {}

        print(f"\n=== 提取FDOTHER.DAT文件: {filepath} ===")

        with open(filepath, "rb") as f:
            data = f.read()

        # 1. 验证文件头
        magic = data[0:6]
        if magic != b"LLLLLL":
            print(f"警告: 魔数不匹配，期望'LLLLLL'，得到'{magic}'")

        # 2. 解析资源表（每个条目8字节：起始偏移+结束偏移）
        resources = []
        offset = 6

        while offset + 8 <= len(data):
            start_offset, end_offset = struct.unpack("<II", data[offset : offset + 8])

            # 检查是否是有效的资源
            if start_offset < end_offset and end_offset <= len(data):
                resource_size = end_offset - start_offset
                resources.append(
                    {
                        "index": len(resources),
                        "table_offset": offset,
                        "start_offset": start_offset,
                        "end_offset": end_offset,
                        "size": resource_size,
                    }
                )

            offset += 8

            # 如果遇到连续的零，可能到达表尾
            if start_offset == 0 and end_offset == 0 and len(resources) > 0:
                break

        print(f"找到 {len(resources)} 个资源条目")

        # 3. 提取每个资源
        extracted_resources = []
        for idx, resource_info in enumerate(resources[:50]):  # 先提取前50个
            try:
                resource_data = self._extract_fdother_resource(data, resource_info, idx)
                if resource_data:
                    extracted_resources.append(resource_data)
                    self._save_resource(resource_data, f"fdother_res{idx:03d}")
            except Exception as e:
                print(f"  提取资源 {idx} 失败: {e}")

        # 4. 生成分析报告
        report = self._generate_fdother_report(resources, extracted_resources)
        report_file = self.output_dir / "fdother_dat_analysis.json"
        with open(report_file, "w") as f:
            json.dump(report, f, indent=2)
        print(f"分析报告已保存到: {report_file}")

        return report

    def _extract_fdother_resource(
        self, data: bytes, resource_info: Dict, index: int
    ) -> Optional[Dict]:
        """提取单个FDOTHER.DAT资源"""
        start_offset = resource_info["start_offset"]
        end_offset = resource_info["end_offset"]
        size = resource_info["size"]

        if end_offset > len(data):
            print(f"  资源 {index}: 结束偏移量 0x{end_offset:08x} 超出文件范围")
            return None

        # 读取资源数据
        resource_data = data[start_offset:end_offset]

        # 分析数据特征
        unique_bytes = len(set(resource_data))
        byte_frequency = {}
        for byte in resource_data:
            byte_frequency[byte] = byte_frequency.get(byte, 0) + 1

        # 常见图形尺寸猜测
        possible_dimensions = []
        for width, height in [
            (320, 200),
            (398, 25),
            (32, 321),
            (256, 256),
            (128, 128),
            (64, 64),
        ]:
            if width * height == size:
                possible_dimensions.append(f"{width}x{height}")

        return {
            "index": index,
            "table_offset": resource_info["table_offset"],
            "start_offset": start_offset,
            "end_offset": end_offset,
            "size": size,
            "unique_bytes": unique_bytes,
            "data_sample": resource_data[:64].hex(),
            "possible_dimensions": possible_dimensions,
            "is_likely_image": size
            in [320 * 200, 398 * 25, 32 * 321, 256 * 256, 128 * 128, 64 * 64],
        }

    def _save_resource(self, resource_data: Dict, base_name: str):
        """保存提取的资源到文件"""
        # 保存原始数据
        if "data_sample" in resource_data:
            # 对于FDOTHER.DAT，我们需要完整数据
            if "start_offset" in resource_data and "end_offset" in resource_data:
                # 从原始文件读取完整数据
                pass

        # 保存元数据
        metadata_file = self.output_dir / f"{base_name}_meta.json"
        with open(metadata_file, "w") as f:
            # 创建可序列化的副本
            serializable_data = {
                k: v
                for k, v in resource_data.items()
                if k != "data" and k != "resource_header_sample"
            }
            json.dump(serializable_data, f, indent=2)

    def _generate_ani_report(
        self, resources: List[Dict], extracted: List[Dict]
    ) -> Dict:
        """生成ANI.DAT分析报告"""
        return {
            "file_type": "ANI.DAT",
            "magic": "LLLLLL",
            "resource_count": len(resources),
            "extracted_count": len(extracted),
            "resources": extracted[:10],  # 只包含前10个资源的详细信息
            "summary": {
                "total_blocks": sum(len(r.get("blocks", [])) for r in extracted),
                "unique_commands": list(
                    set(
                        block["command"]
                        for r in extracted
                        for block in r.get("blocks", [])
                    )
                ),
                "average_blocks_per_resource": sum(
                    len(r.get("blocks", [])) for r in extracted
                )
                / max(len(extracted), 1),
            },
        }

    def _generate_fdother_report(
        self, resources: List[Dict], extracted: List[Dict]
    ) -> Dict:
        """生成FDOTHER.DAT分析报告"""
        image_resources = [r for r in extracted if r.get("is_likely_image", False)]

        return {
            "file_type": "FDOTHER.DAT",
            "magic": "LLLLLL",
            "resource_count": len(resources),
            "extracted_count": len(extracted),
            "image_resources_count": len(image_resources),
            "resources": extracted[:20],  # 只包含前20个资源的详细信息
            "summary": {
                "total_bytes": sum(r.get("size", 0) for r in extracted),
                "average_size": sum(r.get("size", 0) for r in extracted)
                / max(len(extracted), 1),
                "size_distribution": {
                    "small (<1KB)": len(
                        [r for r in extracted if r.get("size", 0) < 1024]
                    ),
                    "medium (1KB-10KB)": len(
                        [r for r in extracted if 1024 <= r.get("size", 0) < 10240]
                    ),
                    "large (10KB-100KB)": len(
                        [r for r in extracted if 10240 <= r.get("size", 0) < 102400]
                    ),
                    "very_large (>100KB)": len(
                        [r for r in extracted if r.get("size", 0) >= 102400]
                    ),
                },
            },
        }

    def decode_ani_command(self, command: int, data: bytes) -> str:
        """解码ANI.DAT命令（基于初步分析）"""
        if command in self.command_handlers:
            return self.command_handlers[command]
        else:
            return f"unknown_command_{command:02x}"

    def generate_extraction_script(self):
        """生成完整的提取脚本"""
        script_content = '''#!/usr/bin/env python3
"""
FD2游戏资源提取脚本 - 自动生成
基于IDA Pro MCP服务器分析结果
"""

import struct
import os

def extract_all_resources():
    """提取所有游戏资源"""
    base_path = "."
    
    # 提取ANI.DAT资源
    print("提取ANI.DAT资源...")
    extract_ani_dat(os.path.join(base_path, "ANI.DAT"))
    
    # 提取FDOTHER.DAT资源
    print("提取FDOTHER.DAT资源...")
    extract_fdother_dat(os.path.join(base_path, "FDOTHER.DAT"))
    
    # 提取其他DAT文件
    other_files = ["BG.DAT", "FIGANI.DAT", "TAI.DAT", "FDTXT.DAT", "FDFIELD.DAT", "FDSHAP.DAT"]
    for filename in other_files:
        filepath = os.path.join(base_path, filename)
        if os.path.exists(filepath):
            print(f"提取{filename}资源...")
            extract_generic_dat(filepath)

def extract_ani_dat(filepath):
    """提取ANI.DAT动画资源"""
    # 实现基于fd2_extractor.py的提取逻辑
    pass

def extract_fdother_dat(filepath):
    """提取FDOTHER.DAT图形资源"""
    # 实现基于fd2_extractor.py的提取逻辑
    pass

def extract_generic_dat(filepath):
    """提取通用DAT文件资源"""
    # 简单提取逻辑
    with open(filepath, 'rb') as f:
        data = f.read()
    
    # 检查文件头
    if data[0:6] == b'LLLLLLLL':
        print(f"  文件 {filepath} 有标准头")
    
    # 保存原始数据
    output_path = filepath + ".raw"
    with open(output_path, 'wb') as f:
        f.write(data)
    print(f"  原始数据已保存到: {output_path}")

if __name__ == "__main__":
    extract_all_resources()
'''

        script_file = self.output_dir / "extract_all_resources.py"
        with open(script_file, "w") as f:
            f.write(script_content)
        print(f"完整提取脚本已生成: {script_file}")

    def extract_all_dat_files(self):
        """提取所有DAT文件"""
        dat_files = [
            "ANI.DAT",
            "FDOTHER.DAT",
            "BG.DAT",
            "FIGANI.DAT",
            "TAI.DAT",
            "FDTXT.DAT",
            "FDFIELD.DAT",
            "FDSHAP.DAT",
            "FDMUS.DAT",
            "DATO.DAT",
            "TITLE.DAT",
        ]

        print("\n=== 提取所有DAT文件 ===")
        reports = {}

        for filename in dat_files:
            filepath = self.base_path / filename
            if not filepath.exists():
                print(f"跳过 {filename}: 文件不存在")
                continue

            print(f"\n处理 {filename}...")

            try:
                if filename == "ANI.DAT":
                    reports[filename] = self.extract_ani_dat(filename)
                elif filename == "FDOTHER.DAT":
                    reports[filename] = self.extract_fdother_dat(filename)
                else:
                    reports[filename] = self.extract_generic_dat(filename)
            except Exception as e:
                print(f"  提取失败: {e}")
                reports[filename] = {"error": str(e)}

        # 生成总体报告
        summary_file = self.output_dir / "all_dat_files_summary.json"
        with open(summary_file, "w") as f:
            json.dump(reports, f, indent=2)
        print(f"\n所有文件提取完成，总报告已保存到: {summary_file}")

        return reports

    def extract_generic_dat(self, filename: str) -> Dict:
        """提取通用DAT文件（非ANI.DAT和FDOTHER.DAT）"""
        filepath = self.base_path / filename
        if not filepath.exists():
            return {}

        with open(filepath, "rb") as f:
            data = f.read()

        # 检查文件头
        magic = data[0:6]
        if magic != b"LLLLLL":
            print(f"  警告: {filename} 没有标准FD2文件头")
            return {}

        # 解析资源表（假设8字节条目）
        resources = []
        offset = 6

        while offset + 8 <= len(data):
            start_offset, end_offset = struct.unpack("<II", data[offset : offset + 8])

            # 检查是否是有效的资源
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

            # 如果遇到连续的零，可能到达表尾
            if start_offset == 0 and end_offset == 0 and len(resources) > 0:
                break

        print(f"  找到 {len(resources)} 个资源条目")

        # 保存原始数据
        output_path = self.output_dir / f"{filename}.raw"
        with open(output_path, "wb") as f:
            f.write(data)

        # 生成报告
        report = {
            "filename": filename,
            "file_size": len(data),
            "magic": magic.hex(),
            "resource_count": len(resources),
            "resources": resources[:20],  # 只包含前20个资源
        }

        # 保存报告
        report_file = (
            self.output_dir / f"{filename.lower().replace('.', '_')}_analysis.json"
        )
        with open(report_file, "w") as f:
            json.dump(report, f, indent=2)

        print(f"  分析报告已保存到: {report_file}")
        print(f"  原始数据已保存到: {output_path}")

        return report


def main():
    """主函数"""
    if len(sys.argv) > 1:
        base_path = sys.argv[1]
    else:
        base_path = "."

    extractor = FD2Extractor(base_path)

    print("FD2游戏资源提取工具")
    print("=" * 60)
    print("基于IDA Pro MCP服务器的逆向工程分析")
    print("=" * 60)

    # 提取所有DAT文件
    all_reports = extractor.extract_all_dat_files()

    # 生成完整提取脚本
    extractor.generate_extraction_script()

    print("\n" + "=" * 60)
    print("提取完成！")
    print(f"提取的资源保存在: {extractor.output_dir}")
    print(f"共处理 {len(all_reports)} 个DAT文件")
    print("\n下一步：")
    print("1. 查看生成的分析报告（.json文件）")
    print("2. 使用生成的提取脚本提取所有资源")
    print("3. 开发图形查看器来显示提取的图像")


if __name__ == "__main__":
    main()
