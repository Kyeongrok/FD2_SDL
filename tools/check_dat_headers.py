#!/usr/bin/env python3
"""
检查所有DAT文件的文件头，了解格式相似性
"""

import os
import sys


def check_dat_files():
    """检查当前目录中的所有DAT文件"""
    dat_files = [f for f in os.listdir(".") if f.endswith(".DAT")]

    print("FD2游戏DAT文件格式检查")
    print("=" * 60)

    for filename in sorted(dat_files):
        try:
            with open(filename, "rb") as f:
                header = f.read(64)  # 读取前64字节
                file_size = os.path.getsize(filename)

            # 检查魔数
            magic = header[:6]
            if magic == b"LLLLLL":
                magic_str = "LLLLLL (标准FD2格式)"
                # 检查是否有资源表
                # 资源表从偏移6开始，尝试解析几个条目
                entries = []
                offset = 6
                while offset + 8 <= len(header):
                    start_offset = int.from_bytes(header[offset : offset + 4], "little")
                    end_offset = int.from_bytes(
                        header[offset + 4 : offset + 8], "little"
                    )
                    if start_offset < end_offset and end_offset <= file_size:
                        entries.append((start_offset, end_offset))
                    offset += 8
                    if len(entries) >= 3:  # 只检查前3个条目
                        break

                if entries:
                    resource_info = f"  资源表条目: {len(entries)} 个有效条目"
                else:
                    resource_info = "  无有效资源表条目"
            else:
                magic_str = f"未知格式: {magic.hex()}"
                resource_info = ""

            # 检查字符串
            strings_found = []
            for i in range(len(header) - 10):
                try:
                    text = header[i : i + 10].decode("ascii", errors="ignore")
                    if text.isprintable() and len(text.strip()) > 3:
                        strings_found.append(text.strip())
                except:
                    pass

            strings_info = ""
            if strings_found:
                strings_info = f"  发现字符串: {', '.join(strings_found[:3])}"

            print(f"\n{filename}:")
            print(f"  大小: {file_size:,} 字节")
            print(f"  魔数: {magic_str}")
            if resource_info:
                print(f"  {resource_info}")
            if strings_info:
                print(f"  {strings_info}")

        except Exception as e:
            print(f"\n{filename}: 读取错误 - {e}")


if __name__ == "__main__":
    check_dat_files()
