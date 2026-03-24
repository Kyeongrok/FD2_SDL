# FDOTHER.DAT RLE压缩格式 - 最终确认

## RLE代码格式

每个代码字节格式: `ttnnnnnn`
- `tt` = 类型 (2位)
- `nnnnnn` = 数量 = (code & 0x3F) + 1

## 四种类型

| 类型 | 位模式 | 说明 |
|------|--------|------|
| 0 | 00xxxxxx | 填充：用下一字节的值填充num字节 |
| 1 | 01xxxxxx | 填充：用下一字节的值填充num字节 |
| 2 | 10xxxxxx | 填充：用下一字节的值填充num字节 |
| 3 | 11xxxxxx | **特殊填充**：用代码字节本身的值填充num字节（不读额外字节）|

## 关键发现

**类型3的特殊行为**:
- 常规理解: 跳过/不写入
- **实际行为**: 使用代码字节的值填充（代码即填充值）
- 例如: `0xFF` = 用0xFF填充64字节

这解释了为什么res_59等资源开头全是0xFF，结果是白色（palette index 255）。

## 解压算法

```c
void decompress_rle(byte* src, int src_size, byte* dst, int stride) {
    word count = *(word*)src;      // 每行字节数
    word num_rows = *(word*)(src + 2);  // 行数
    
    src += 4;  // 跳过4字节header
    
    for (int row = 0; row < num_rows; row++) {
        int remaining = count;
        
        while (remaining > 0) {
            byte code = *src++;
            int num = (code & 0x3F) + 1;
            
            int type = (code >> 6) & 0x3;
            
            if (type != 3) {
                // type 0,1,2: 用下一字节填充
                byte fill = *src++;
                memset(dst, fill, num);
            } else {
                // type 3: 用代码字节填充（代码即值）
                memset(dst, code, num);
            }
            dst += num;
            remaining -= num;
        }
        dst += stride - count;  // 跳到下一行
    }
}
```

## 图像处理

- 所有图像需要垂直翻转（Y轴镜像）
- Palette在资源0，offset 422，768字节
- 6-bit到8-bit转换: `val8 = (val6 << 2) | (val6 >> 4)`
