# ANI.DAT 启动动画分析

## 概述

本文档分析游戏《炎龙骑士团2》中 `ANI.DAT` 文件的加载和播放机制，包括启动过程中所有相关函数的调用关系和伪代码实现。

## 目录

1. [ANI.DAT 文件格式](#ani.dat-文件格式)
2. [函数调用关系](#函数调用关系)
3. [核心函数分析](#核心函数分析)
4. [启动流程时序](#启动流程时序)
5. [伪代码实现](#伪代码实现)

---

## ANI.DAT 文件格式

### 文件头结构
```
偏移量    大小    描述
0x00      4       资源0的偏移量
0x04      4       资源1的偏移量
0x06      4*      资源表 (每个资源4字节)
```

### 资源头结构 (173字节)
```
偏移量    大小    描述
0xA5      2       Block数量 (word)
...
```

### Block头结构 (6字节)
```
偏移量    大小    描述
0x00      2       Block数据大小
0x02      2       命令数量
```

### 命令类型 (0-9)

| 命令 | 函数 | 描述 |
|------|------|------|
| 0 | do_h0 | 设置调色板为单一颜色 (96项) |
| 1 | do_h1 | 直接复制调色板 (768字节) |
| 2 | do_h2 | RLE压缩调色板解压 |
| 3 | do_h3 | 部分调色板更新 |
| 4 | do_h4 | 用单一颜色填充屏幕 |
| 5 | do_h5 | 直接复制屏幕 (64000字节) |
| 6 | do_h6 | RLE压缩屏幕解压 |
| 7 | do_h7 | 屏幕区域填充 |
| 8 | do_h8 | 设置单个像素 |
| 9 | do_h9 | 屏幕数据块复制 |

---

## 函数调用关系

### ANI.DAT 播放核心调用链

```
play_ani_resource()
├── fopen("ANI.DAT")
├── read header (173 bytes)
├── for each block:
│   ├── read block header (6 bytes)
│   ├── read block data
│   ├── memset(ani_screen_buf, 0, 64000)
│   ├── memset(ani_palette_buf, 0, 768)
│   ├── decode_ani_block()
│   │   └── run_ani_command()
│   │       ├── do_h0()  - 设置调色板为单一颜色
│   │       ├── do_h1()  - 复制调色板
│   │       ├── do_h2()  - RLE压缩调色板
│   │       ├── do_h3()  - 部分调色板更新
│   │       ├── do_h4()  - 用单一颜色填充屏幕
│   │       ├── do_h5()  - 复制屏幕
│   │       ├── do_h6()  - RLE压缩屏幕
│   │       ├── do_h7()  - 屏幕区域填充
│   │       ├── do_h8()  - 设置单个像素
│   │       └── do_h9()  - 屏幕数据块复制
│   ├── delay_ms(frame_delay)
│   └── check_key_input()
└── fclose()
```

### 原始IDA逆向函数对应关系

| C实现函数 | IDA函数地址 | 描述 |
|----------|------------|------|
| play_ani_resource() | sub_20421 | 主播放函数 |
| decode_ani_block() | sub_36FF4 | Block解码调度器 |
| do_h0() | sub_36E3D | 命令0处理 |
| do_h1() | sub_36E57 | 命令1处理 |
| do_h2() | sub_36E65 | 命令2处理 |
| do_h3() | sub_36EA7 | 命令3处理 |
| do_h4() | sub_36EE0 | 命令4处理 |
| do_h5() | sub_36F08 | 命令5处理 |
| do_h6() | sub_36F24 | 命令6处理 |
| do_h7() | sub_36F69 | 命令7处理 |
| do_h8() | sub_36F82 | 命令8处理 |
| do_h9() | sub_36FAC | 命令9处理 |

---

## 核心函数分析

### 1. play_ani_resource() - 主播放函数

**地址:** 0x20421 (sub_20421)

**功能:** 播放ANI.DAT中指定的动画资源

**参数:**
- res_idx: 资源索引 (0-8)
- frame_delay: 帧延迟 (毫秒)
- wait_key: 是否等待按键

**调用者:**
- sub_1F894 (标题画面)
- sub_1F81E (菜单处理)
- sub_24336 (游戏启动)

### 2. decode_ani_block() - Block解码器

**地址:** 0x36FF4 (sub_36FF4)

**功能:** 解析Block数据并执行命令序列

**实现方式:** 使用跳转表调度命令处理函数

### 3. run_ani_command() - 命令调度器

**功能:** 根据命令号调用对应的处理函数

---

## 启动流程时序

### 阶段1: 初始化
```
main()
├── init_game_machine()
├── load_resources()
│   └── load_ani_cached(0)  // 加载标题动画资源信息
└── run_full_startup_sequence()
```

### 阶段2: 启动动画播放
```
run_full_startup_sequence()
├── Phase 1: 播放开场动画
│   ├── load_ani_cached(0)
│   ├── for i in range(30):
│   │   └── decode_ani_block(i)  // 逐帧解码
│   └── 延迟处理
│
├── Phase 2: 播放资源3动画 (90ms/帧)
│   └── play_ani_resource(3, 90, 1)
│
├── Phase 3: 条形动画 (535帧)
│   └── bar_animation()
│
├── Phase 4: 淡入淡出效果
│   └── fade_effect()
│
└── Phase 5: 播放资源1动画 (15ms/帧)
    └── play_ani_resource(1, 15, 1)
```

### 阶段3: 标题画面
```
game_loop()
├── if state == STATE_TITLE:
│   └── update_ani_frame()
│       ├── load_ani_cached(g_ani_res_idx)
│       └── decode_ani_block(frame_idx)
└── render_state()
```

---

## 伪代码实现

### play_ani_resource() - ANI播放主函数

```c
/**
 * 播放ANI.DAT中的指定资源
 * @param res_idx 资源索引
 * @param frame_delay 帧间延迟(ms)
 * @param wait_key 是否等待按键
 * @return 成功返回0，失败返回-1
 */
int play_ani_resource(int res_idx, int frame_delay, int wait_key)
{
    FILE* fp = fopen("ANI.DAT", "rb");
    if (!fp) return -1;
    
    // 定位到资源偏移表
    fseek(fp, 4 * res_idx + 6, SEEK_SET);
    dword offset;
    fread(&offset, 4, 1, fp);
    
    // 读取资源头 (173字节)
    fseek(fp, offset, SEEK_SET);
    byte header[173];
    fread(header, 1, 173, fp);
    
    // 获取Block数量 (偏移0xA5)
    word block_count = *(word*)(header + 165);
    
    // 分配缓冲区
    byte* screen_buf = malloc(64000);
    byte* palette_buf = malloc(768);
    
    // 逐Block处理
    for (int i = 0; i < block_count; i++)
    {
        // 读取Block头 (6字节: size + cmd_count)
        byte block_header[6];
        fread(block_header, 1, 6, fp);
        
        word size = *(word*)(block_header + 0);
        word cmd_count = *(word*)(block_header + 2);
        
        if (size == 0 || cmd_count == 0) continue;
        
        // 读取Block数据
        byte* block_data = malloc(size);
        fread(block_data, 1, size, fp);
        
        // 清空缓冲区
        memset(screen_buf, 0, 64000);
        memset(palette_buf, 0, 768);
        
        // 解码Block
        decode_ani_block(block_data, size, cmd_count, screen_buf, palette_buf);
        
        // 渲染到屏幕
        render_frame(screen_buf);
        apply_palette(palette_buf);
        
        // 帧间延迟
        delay_ms(frame_delay);
        
        // 检查按键
        if (wait_key && key_pressed())
            break;
        
        free(block_data);
    }
    
    free(screen_buf);
    free(palette_buf);
    fclose(fp);
    return 0;
}
```

### decode_ani_block() - Block解码器

```c
/**
 * 解码ANI Block
 * @param data Block数据
 * @param size 数据大小
 * @param cmd_count 命令数量
 * @param screen 屏幕缓冲区
 * @param palette 调色板缓冲区
 */
void decode_ani_block(byte* data, int size, int cmd_count, 
                      byte* screen, byte* palette)
{
    int pos = 0;
    
    for (int i = 0; i < cmd_count && pos < size; i++)
    {
        byte cmd = data[pos++];
        
        if (cmd < 10)
        {
            int consumed = run_ani_command(cmd, data + pos, 
                                          size - pos, screen, palette);
            pos += consumed;
        }
    }
}
```

### run_ani_command() - 命令调度器

```c
/**
 * 执行ANI命令
 * @param cmd 命令号 (0-9)
 * @param data 命令数据
 * @param size 剩余数据大小
 * @param screen 屏幕缓冲区
 * @param palette 调色板缓冲区
 * @return 消耗的字节数
 */
int run_ani_command(byte cmd, byte* data, int size, 
                    byte* screen, byte* palette)
{
    switch (cmd)
    {
        case 0: return do_h0(data, palette);
        case 1: return do_h1(data, size, palette);
        case 2: return do_h2(data, size, palette);
        case 3: return do_h3(data, size, palette);
        case 4: return do_h4(data, screen);
        case 5: return do_h5(data, screen);
        case 6: return do_h6(data, size, screen);
        case 7: return do_h7(data, size, screen);
        case 8: return do_h8(data, size, screen);
        case 9: return do_h9(data, size, screen);
        default: return 0;
    }
}
```

### do_h0() - 设置调色板为单一颜色

```c
/**
 * 命令0: 设置调色板前96项为单一颜色
 * @param data 1字节颜色值
 * @param palette 调色板缓冲区
 * @return 消耗字节数 (1)
 */
int do_h0(byte* data, byte* palette)
{
    byte b = data[0];
    dword color = (b << 16) | (b << 8) | b;  // RGB相同
    
    dword* p = (dword*)palette;
    for (int i = 0; i < 96; i++)
        p[i] = color;
    
    return 1;
}
```

### do_h1() - 直接复制调色板

```c
/**
 * 命令1: 直接复制调色板数据
 * @param data 调色板数据 (最多768字节)
 * @param size 数据大小
 * @param palette 调色板缓冲区
 * @return 消耗字节数
 */
int do_h1(byte* data, int size, byte* palette)
{
    int copy_size = min(size, 768);
    memcpy(palette, data, copy_size);
    return copy_size;
}
```

### do_h2() - RLE压缩调色板

```c
/**
 * 命令2: RLE压缩调色板解压
 * @param data RLE压缩数据
 * @param size 数据大小
 * @param palette 调色板缓冲区
 * @return 消耗字节数
 */
int do_h2(byte* data, int size, byte* palette)
{
    int filled = 0;
    int pos = 0;
    
    while (filled < 768 && pos < size)
    {
        byte b = data[pos++];
        
        if ((b & 0xC0) == 0xC0)  // RLE标记
        {
            int run = b & 0x3F;
            byte value = data[pos++];
            int count = (run >> 1) + (run & 1);
            
            for (int i = 0; i < count && filled < 768; i++)
                palette[filled++] = value;
        }
        else
        {
            palette[filled++] = b;
        }
    }
    
    return pos;
}
```

### do_h3() - 部分调色板更新

```c
/**
 * 命令3: 部分调色板更新
 * @param data 更新数据
 * @param size 数据大小
 * @param palette 调色板缓冲区
 * @return 消耗字节数
 */
int do_h3(byte* data, int size, byte* palette)
{
    int pos = 0;
    int count = data[pos++];
    
    for (int i = 0; i < count && pos + 2 <= size; i++)
    {
        int offset = data[pos++];
        int copy_bytes = data[pos++];
        int actual_offset = offset * 3;
        int actual_copy = (copy_bytes * 3) >> 1;
        
        if (actual_offset + actual_copy <= 768 && pos + actual_copy <= size)
        {
            memcpy(palette + actual_offset, data + pos, actual_copy);
        }
        pos += actual_copy;
    }
    
    return pos;
}
```

### do_h4() - 用单一颜色填充屏幕

```c
/**
 * 命令4: 用单一颜色填充屏幕
 * @param data 1字节颜色值
 * @param screen 屏幕缓冲区
 * @return 消耗字节数 (1)
 */
int do_h4(byte* data, byte* screen)
{
    byte b = data[0];
    dword color = (b << 16) | (b << 8) | b;
    
    dword* p = (dword*)screen;
    for (int i = 0; i < 10000; i++)  // 前40000字节用dword填充
        p[i] = color;
    
    for (int i = 40000; i < 64000; i++)  // 剩余字节
        screen[i] = b;
    
    return 1;
}
```

### do_h5() - 直接复制屏幕

```c
/**
 * 命令5: 直接复制屏幕数据 (64000字节)
 * @param data 屏幕数据
 * @param screen 屏幕缓冲区
 * @return 消耗字节数 (64000)
 */
int do_h5(byte* data, byte* screen)
{
    memcpy(screen, data, 64000);
    return 64000;
}
```

### do_h6() - RLE压缩屏幕

```c
/**
 * 命令6: RLE压缩屏幕解压
 * @param data RLE压缩数据
 * @param size 数据大小
 * @param screen 屏幕缓冲区
 * @return 消耗字节数
 */
int do_h6(byte* data, int size, byte* screen)
{
    int filled = 0;
    int pos = 0;
    
    while (filled < 64000 && pos < size)
    {
        byte b = data[pos++];
        
        if ((b & 0xC0) == 0xC0)  // RLE标记
        {
            int run = b & 0x3F;
            byte value = data[pos++];
            int words = run >> 1;
            int bytes = run & 1;
            
            // 填充word (2字节)
            for (int i = 0; i < words && filled < 64000; i++)
            {
                screen[filled++] = value;
                if (filled < 64000) screen[filled++] = value;
            }
            
            // 填充单字节
            for (int i = 0; i < bytes && filled < 64000; i++)
                screen[filled++] = value;
        }
        else
        {
            screen[filled++] = b;
        }
    }
    
    return pos;
}
```

### do_h7() - 屏幕区域填充

```c
/**
 * 命令7: 屏幕区域填充
 * @param data 填充数据
 * @param size 数据大小
 * @param screen 屏幕缓冲区
 * @return 消耗字节数
 */
int do_h7(byte* data, int size, byte* screen)
{
    int pos = 0;
    
    // 读取2字节count
    int count_lo = data[pos++];
    int count_hi = data[pos++];
    int count = count_lo | (count_hi << 8);
    
    for (int i = 0; i < count && pos + 3 <= size; i++)
    {
        int off_lo = data[pos++];
        int off_hi = data[pos++];
        int run = data[pos++];
        int value = data[pos++];
        int offset = off_lo | (off_hi << 8);
        
        if (offset >= 0 && offset < 64000)
        {
            int words = run >> 1;
            int bytes = run & 1;
            
            for (int j = 0; j < words && offset < 64000; j++)
            {
                screen[offset++] = value;
                if (offset < 64000) screen[offset++] = value;
            }
            
            for (int j = 0; j < bytes && offset < 64000; j++)
                screen[offset++] = value;
        }
    }
    
    return pos;
}
```

### do_h8() - 设置单个像素

```c
/**
 * 命令8: 设置多个单像素
 * @param data 像素数据
 * @param size 数据大小
 * @param screen 屏幕缓冲区
 * @return 消耗字节数
 */
int do_h8(byte* data, int size, byte* screen)
{
    int pos = 0;
    
    // 读取2字节count
    int count_lo = data[pos++];
    int count_hi = data[pos++];
    int count = count_lo | (count_hi << 8);
    
    for (int i = 0; i < count && pos + 2 <= size; i++)
    {
        int off_lo = data[pos++];
        int off_hi = data[pos++];
        int offset = off_lo | (off_hi << 8);
        byte value = data[pos++];
        
        if (offset >= 0 && offset < 64000)
            screen[offset] = value;
    }
    
    return pos;
}
```

### do_h9() - 屏幕数据块复制

```c
/**
 * 命令9: 屏幕数据块复制
 * @param data 块数据
 * @param size 数据大小
 * @param screen 屏幕缓冲区
 * @return 消耗字节数
 */
int do_h9(byte* data, int size, byte* screen)
{
    int pos = 0;
    int count = data[pos++];
    
    for (int i = 0; i < count && pos + 4 <= size; i++)
    {
        int off_lo = data[pos++];
        int off_hi = data[pos++];
        int stride_lo = data[pos++];
        int stride_hi = data[pos++];
        
        int offset = off_lo | (off_hi << 8);
        int stride = stride_lo | (stride_hi << 8);
        
        if (offset >= 0 && stride > 0 && 
            offset + stride <= 64000 && pos + stride <= size)
        {
            memcpy(screen + offset, data + pos, stride);
        }
        pos += stride;
    }
    
    return pos;
}
```

---

## 全局变量

| 变量名 | 地址 | 类型 | 描述 |
|--------|------|------|------|
| ani_screen_buf | - | byte[64000] | 屏幕缓冲区 |
| ani_palette_buf | - | byte[768] | 调色板缓冲区 |
| g_ani_res_idx | - | int | 当前ANI资源索引 |
| g_ani_frame | 0x138 | int | 当前帧号 |
| g_ani_block_count | - | int | Block数量 |
| g_ani_blocks | - | ANIBlockInfo* | Block信息缓存 |
| g_ani_cache | - | byte* | Block数据缓存 |

---

## 总结

ANI.DAT动画系统采用基于命令的帧描述格式：

1. **文件结构**: 资源表 → 资源头 → Block头 → Block数据
2. **命令系统**: 10种命令处理调色板和屏幕数据
3. **压缩方式**: 支持RLE压缩和直接数据复制
4. **播放机制**: 每帧独立解码，支持帧间延迟和按键中断

启动流程中的ANI资源使用：
- **资源0**: 标题动画
- **资源1**: 菜单动画 (15ms/帧)
- **资源3**: 开场动画 (90ms/帧)
