# FDOTHER.DAT 图像提取最终报告

## 问题修复

### 1. 图像上下颠倒
**解决方案**: 提取后需要垂直翻转图像

### 2. RLE压缩格式
**最终确认的格式**:
| 类型 | 位模式 | 说明 |
|------|--------|------|
| 00 | 00xxxxxx | 用下一字节填充 num 字节 |
| 01 | 01xxxxxx | 复制 num 字节（字面复制） |
| 10 | 10xxxxxx | 用下一字节填充 num 字节 |
| 11 | 11xxxxxx | 跳过（仅前进指针，remaining-=num） |

num = (code & 0x3F) + 1

## 提取结果

| 资源 | 大小 | 类型 | 状态 |
|------|------|------|------|
| 11 | 53587 | RLE | ✓ 完整游戏背景 |
| 15 | 64004 | RAW | ✓ 标题画面 |
| 55 | 64004 | RAW | ✓ 背景图 |
| 56 | 13609 | RLE | ✓ 有内容 |
| 59 | 3278 | RLE | 部分（叠加层？） |
| 60 | 2849 | RLE | 部分（叠加层？） |
| 61 | 54727 | RLE | ✓ 完整游戏背景 |
| 62 | 43434 | RLE | ✓ 有内容 |
| 74 | 6585 | RLE | 部分（蓝色区域） |
| 75 | 37776 | RLE | ✓ 有内容 |
| 97 | 39358 | RLE | ✓ 有内容 |
| 100 | 15746 | RLE | 部分（叠加层？） |

## 输出文件

- `images/res_*_v6.png` - 最终提取的PNG图像
- `src/fd2_rle.c` - 更新的RLE解压函数
- `src/fdother_extract.c` - 完整的提取工具

## RLE解压算法

```c
void decompress_rle(byte* src, int src_size, byte* dst, int stride) {
    word count = *(word*)src;      // 每行字节数
    word num_rows = *(word*)(src + 2);  // 行数
    
    src += 4;
    int pos = 0;
    
    for (int row = 0; row < num_rows; row++) {
        int remaining = count;
        
        while (remaining > 0 && pos < src_size) {
            byte code = src[pos++];
            int num = (code & 0x3F) + 1;
            if (num > remaining) num = remaining;
            
            int type = (code >> 6) & 0x3;
            
            switch(type) {
                case 0: // 00: fill
                case 2: // 10: fill
                    byte fill = src[pos++];
                    memset(dst, fill, num);
                    dst += num;
                    break;
                case 1: // 01: literal
                    memcpy(dst, src + pos, num);
                    pos += num;
                    dst += num;
                    break;
                case 3: // 11: skip
                    dst += num;
                    break;
            }
            remaining -= num;
        }
        dst += stride - count;
    }
}
```
