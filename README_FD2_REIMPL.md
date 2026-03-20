# FD2.exe 重新实现总结

## 可执行文件
- `fd2_game_sdl.exe` (118KB) - 主要实现，SDL2 图形模式
- `fd2_game.exe` (118KB) - 控制台 ASCII 模式
- `fd2_reimpl_sdl.exe` (171KB) - 参考实现（SDL2）
- `fd2_reimpl.exe` - 参考实现（控制台）

## 构建命令
```bash
# 非SDL（控制台）
gcc -Wall -Wextra -std=c99 -g -o fd2_game fd2_game.c -lm

# SDL2 图形模式
gcc -Wall -Wextra -std=c99 -g -DSDL_MAIN_HANDLED -DUSE_SDL -Iinclude -o fd2_game_sdl fd2_game.c fd2_sdl_renderer_impl.c -Llib -lSDL2main -lSDL2 -lm
```

## ANI.DAT 格式（IDA Pro 逆向工程）

### 文件结构
- Header: 6字节 "LLLLLL" + 资源表
- 资源偏移 = 4 * resource_index + 6 from file start
- ANI.DAT 资源0: 173字节 header (offset 165 = block count WORD)
- Block: [size(WORD), cmd_count(WORD), unk(WORD), unk(WORD)] + 命令数据

### ANI 命令处理器
调度器: sub_36FF4 (0x36ff4), 函数表 at 0x5276a

**调色板命令 (buf[768], 前256字节 = screen_buffer[0-255]):**
- h0 (0x36E3D): memset32 调色板 768 bytes
- h1 (0x36E57): memcpy 768 字节到调色板
- h2 (0x36E65): RLE 解压到调色板
- h3 (0x36EA7): 调色板补丁 [count][offset*3][len][data]...

**屏幕命令 (screen_buffer[64000]):**
- h4 (0x36EE0): memset32 屏幕 64000 bytes
- h5 (0x36F08): memcpy 64000 字节到屏幕
- h6 (0x36F24): RLE 解压到屏幕
- h7 (0x36F69): 屏幕补丁 [count][offset][value]... 从 off=256 开始
- h8 (0x36F82): 屏幕 memset 补丁 [count][offset][stride][value]
- h9 (0x36FAC): 屏幕 memcpy 补丁 [count][offset][stride][data]

### 关键发现
1. ANI 调色板是 8-bit 值，直接复制到 SDL 调色板（不需要 6→8 bit 转换）
2. 必须每帧应用 ANI 调色板，否则画面变成黑白
3. n655360 = 655360 = 0xA0000 (VGA 显存段)

## 修复的 Bug
1. **present() 渲染错误缓冲** → 使用 fd2_sdl_render_frame(g_render.screen_buffer)
2. **ANI 调色板未应用** → 每帧从 ani_palette_buf 复制 768 字节
3. **ANI h2/h6 指针运算** → 修复 word memset 反向写入
4. **ANI h3 偏移** → 调色板偏移 = off * 3
5. **ANI h7** → 从 screen_buffer[256] 开始写入
6. **SDLK_R/P 编译错误** → 删除不存在的常量

## 游戏状态机
```
STATE_TITLE (1) - ANI 动画，Enter 进入关卡选择
STATE_LEVEL_SELECT (3) - 选择关卡 1-50
STATE_PLAYING (4) - 玩游戏
STATE_PAUSED (5) - 按 P 暂停
STATE_WIN (6) - 过关
```

## 待完成
1. 测试游戏流程
2. 实现真正的拼图机制（使用 FDFIELD/FDSHAP 数据）
3. 实现 FDMUS.DAT 音乐播放（XMI 格式）
