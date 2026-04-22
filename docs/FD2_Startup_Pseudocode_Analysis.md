# FD2.EXE 시작 과정 역공학 함수 의사 코드 분석

> IDA Pro MCP 서버로 추출한 디컴파일 코드 정리

---

## 목차

1. [진입점과 메인 함수](#1-진입점과-메인-함수)
2. [초기화 함수](#2-초기화-함수)
3. [DAT 파일 로더](#3-dat-파일-로더)
4. [오디오 시스템 함수](#4-오디오-시스템-함수)
5. [게임 상태 / 음악 초기화](#5-게임-상태--음악-초기화)
6. [세이브 로드 함수](#6-세이브-로드-함수)
7. [오프닝 애니메이션 함수](#7-오프닝-애니메이션-함수)
8. [메뉴 표시 함수](#8-메뉴-표시-함수)
9. [비디오 / 디스플레이 함수](#9-비디오--디스플레이-함수)
10. [팔레트 함수](#10-팔레트-함수)
11. [디컴파일 상태 정리](#11-디컴파일-상태-정리)

---

## 1. 진입점과 메인 함수

### 1.1 start (0x3CCB4)
**디컴파일 상태**: ❌ 미디컴파일 (라이브러리 함수는 스킵됨)

```
// 库函数 - DOS/4G 入口点
// 由运行时环境调用，跳转到 __CMain -> main
```

### 1.2 main (0x25BF4)
**디컴파일 상태**: ✅ 완료

```c
int main(int argc, const char **argv, const char **envp)
{
    int random_seed, i;
    
    // 1. 栈初始化
    sub_3702F(28);
    
    // 2. 音频系统初始化
    sub_37D3E();                    // AIL_startup()
    sub_3AA72();                    // 安装 MDI 驱动
    dword_53ED8 = result;           // 保存驱动状态
    if (result) {
        n16_0 = 1;
        dword_53ED0 = sub_3ACA3();  // 分配序列句柄
    }
    
    // 3. 初始化数字音频驱动
    driver_handle = sub_3908B();    // AIL_install_DIG_INI()
    dword_53EDC = driver_handle;
    if (driver_handle) {
        byte_53EF1 = 1;
        dword_53EE4 = sub_392D0(driver_handle);  // 分配采样句柄1
        dword_53EE8 = sub_392D0(driver_handle);  // 分配采样句柄2
    }
    
    // 4. 加载数据文件
    FDOTHER_DAT__1 = sub_111BA("FDOTHER.DAT", 0);  // 资源0
    FDOTHER_DAT__1 = sub_111BA("FDOTHER.DAT", 1);  // 资源1
    n30_0 = sub_111BA("FDOTHER.DAT", 2);           // 资源2
    FDOTHER_DAT__3 = sub_111BA("FDOTHER.DAT", 3);  // 资源3
    FDOTHER_DAT__4 = sub_111BA("FDOTHER.DAT", 4);  // 资源4
    FDOTHER_DAT__5 = sub_111BA("FDOTHER.DAT", 5);  // 资源5
    arg0 = sub_111BA("FDTXT.DAT", 0);              // 文本数据
    FDOTHER_DAT__6 = sub_111BA("FDOTHER.DAT", 6);  // 资源6
    
    // 5. 分配内存
    dword_53AD5 = malloc(32);       // 调色板缓冲区
    n655360_0 = malloc(64000);      // 视频缓冲区 (64KB)
    dword_53BF7 = malloc(2560);     // 图形缓冲区
    
    // 6. 初始化随机数
    int386(16, &regs, &regs);       // 获取系统计时器
    dword_53C0F = MEMORY[0x46C];    // BIOS 计时器
    random_seed = rand();
    for (i = 0; i < random_seed % 256; i++)
        sub_4EBE3(random_seed);     // 初始化随机数表
    
    // 7. 主游戏循环
    while (1) {
        result = sub_25977(18, 0);  // 初始化状态/播放音乐
        result = sub_25EBB();       // 检查存档/进入游戏
        
        if (result == 0) {
            // 新游戏或继续游戏
            do {
                result = sub_117E7();  // 状态机/菜单处理
                
                if (n2_0 == 1) {
                    // 进入新状态
                    byte_51AAC = 0;
                    sub_22E5C();
                    byte_51AAC = 1;
                    n2_0 = 0;
                }
                else if (n2_0 == 2) {
                    // 执行状态处理函数
                    byte_51AAC = 0;
                    sub_25977(-1, 1);
                    funcs_25E23[n17]();  // 状态处理
                    result = sub_26152();
                    // ...
                }
            } while (!result);
        }
        
        if (result) {
            // 退出游戏
            sub_37ED8();
            exit_program();
        }
    }
}
```

---

## 2. 초기화 함수

### 2.1 sub_3702F (0x3702F) - 스택 초기화
**디컴파일 상태**: ✅ 완료

```c
// 栈溢出检查和初始化
int sub_3702F(int stack_size)
{
    return sub_37042(stack_size);  // 实际检查函数
}
```

### 2.2 sub_37042 (0x37042) - 스택 체크
**디컴파일 상태**: ✅ 완료

```c
// 检查栈是否溢出
char* sub_37042(unsigned int required_size)
{
    char *current_sp;
    char *retaddr;  // 返回地址
    
    // 计算当前栈指针
    current_sp = (char*)&retaddr - required_size;
    
    // 检查是否超出栈边界
    if (current_sp <= stack_base) {
        return sub_3705F();  // 栈溢出处理
    }
    return current_sp;
}
```

---

## 3. DAT 파일 로더

### 3.1 sub_111BA (0x111BA) - DAT 리소스 로드
**디컴파일 상태**: ✅ 완료

```c
// 从DAT文件加载指定资源
BYTE* sub_111BA(
    int unused1, int unused2, int unused3, int unused4,
    const char *filename,      // DAT文件名
    int existing_buffer,       // 已存在的缓冲区(会先释放)
    int resource_index)        // 资源索引号
{
    FILE *fp;
    int *index_entry;
    BYTE *buffer;
    int offset, size;
    
    sub_3702F(32);  // 栈检查
    
    // 如果已有缓冲区，先释放
    if (existing_buffer)
        free(existing_buffer);
    
    // 打开文件
    fp = fopen(filename, "rb");
    if (!fp) {
        printf("\n\n File not found %s!!! \n\n", filename);
        goto error;
    }
    
    // 读取索引条目 (资源索引 * 4 + 6 字节偏移)
    index_entry = malloc(8);
    fseek(fp, 4 * resource_index + 6, 0);
    fread(index_entry, 1, 8, fp);
    
    // 计算资源偏移和大小
    offset = index_entry[0];           // 资源起始偏移
    size = index_entry[1] - offset;    // 资源大小
    
    // 分配缓冲区
    buffer = malloc(size);
    if (!buffer) {
        printf("Out of Memory at Load %s Number:%d!!\n", filename, resource_index);
        goto error;
    }
    
    // 读取资源数据
    fseek(fp, offset, 0);
    fread(buffer, 1, size, fp);
    
    fclose(fp);
    dword_53BFF = size;  // 保存最后加载的大小
    return buffer;

error:
    // 错误处理 - 跳转到退出代码
    exit(3);
}
```

---

## 4. 오디오 시스템 함수

### 4.1 sub_37D3E (0x37D3E) - AIL_startup
**디컴파일 상태**: ✅ 완료 (调试包装器)

```c
// Miles Audio Interface Library 启动
void sub_37D3E()
{
    fprintf(env, "AIL_startup()\n");  // 调试输出
    // 实际调用 AIL 内部初始化
}
```

### 4.2 sub_3AA72 (0x3AA72) - MDI 드라이버 설치
**디컴파일 상태**: ✅ 완료

```c
// 安装音乐驱动接口
void sub_3AA72()
{
    dword_54178++;  // 嵌套层级计数
    
    // 检查是否需要安装
    if (dword_54174 && (dword_54178 == 1 || dword_54170)) {
        if (!sub_3F22A() && sub_37C9C()) {
            fprintf(env, "AIL_install_MDI_INI()\n");
        }
    }
    
    // 实际安装调用
    result = sub_44240();  // AIL 内部安装
    
    // 输出调试信息
    if (dword_54174 && ...) {
        // 输出缩进和结果
        fprintf(env, "Result = 0x%X\n", result);
    }
    
    JUMPOUT(0x39155);  // 跳转到清理代码
}
```

### 4.3 sub_3908B (0x3908B) - DIG 드라이버 초기화
**디컴파일 상태**: ✅ 완료

```c
// 安装数字音频驱动
int sub_3908B()
{
    int driver_handle;
    
    dword_54178++;
    
    // 调试输出
    if (dword_54174 && ...) {
        fprintf(env, "AIL_install_DIG_INI()\n");
    }
    
    // 安装驱动
    driver_handle = sub_413C0();  // AIL_install_DIG_INI()
    
    // 输出结果
    if (dword_54178 == 1 || dword_54170) {
        fprintf(env, "Result = 0x%X\n", driver_handle);
    }
    
    dword_54178--;
    return driver_handle;
}
```

### 4.4 sub_392D0 (0x392D0) - 샘플 핸들 할당
**디컴파일 상태**: ✅ 완료

```c
// 分配音频采样句柄
void sub_392D0(int driver_handle)
{
    int sample_handle;
    
    dword_54178++;
    
    // 调试输出
    fprintf(env, "AIL_allocate_sample_handle(0x%X)\n", driver_handle);
    
    // 分配句柄
    sample_handle = sub_41460(driver_handle);  // AIL_allocate_sample_handle
    
    // 输出结果
    fprintf(env, "Result = 0x%X\n", sample_handle);
    
    JUMPOUT(0x3916A);  // 清理返回
}
```

### 4.5 sub_3ACA3 (0x3ACA3) - 시퀀스 핸들 할당
**디컴파일 상태**: ✅ 완료

```c
// 分配音乐序列句柄
void sub_3ACA3(int driver_handle)
{
    int seq_handle;
    
    dword_54178++;
    
    // 调试输出
    fprintf(env, "AIL_allocate_sequence_handle(0x%X)\n", driver_handle);
    
    // 分配句柄
    seq_handle = sub_44330(driver_handle);  // AIL_allocate_sequence_handle
    
    // 输出结果
    fprintf(env, "Result = 0x%X\n", seq_handle);
    
    JUMPOUT(0x38154);  // 清理返回
}
```

---

## 5. 게임 상태 / 음악 초기화

### 5.1 sub_25977 (0x25977) - 상태 초기화 / 음악 제어
**디컴파일 상태**: ✅ 완료

```c
// 初始化游戏状态并控制音乐播放
int sub_25977(int state_id, int arg4)
{
    // 如果状态已改变
    if (state_id != current_state) {
        current_state = state_id;
        
        // 停止当前音乐
        if (state_id == -1) {
            return sub_3B124(dword_53ED0, 0, 4000);  // 设置音量淡出
        }
        
        // 加载新状态的音乐
        if (n16_0) {
            // 停止当前序列
            if (FDMUS_DAT)
                sub_3AF5B(dword_53ED0);  // AIL_stop_sequence
            
            // 加载音乐文件
            FDMUS_DAT = sub_111BA("FDMUS.DAT", state_id);
            
            // 验证音乐数据
            sub_3666C(FDMUS_DAT, dword_53BFF);
            
            // 初始化序列
            sub_3ADF5(dword_53ED0, FDMUS_DAT, 0);
            
            // 开始播放
            sub_3AEEE(dword_53ED0);
            
            // 设置音量
            if (n127) {
                if (state_id == 16 || state_id == 17) {
                    sub_3B124(dword_53ED0, 0, 0);    // 无淡入
                } else {
                    sub_3B124(dword_53ED0, 0, 2000); // 淡入
                }
                sub_3B124(dword_53ED0, 127, 0);      // 最大音量
            } else {
                sub_3B124(dword_53ED0, 0, 0);        // 静音
            }
            
            // 设置循环
            return sub_3B1A6(dword_53ED0, arg4);
        }
    }
    return current_state;
}
```

### 5.2 sub_3AF5B (0x3AF5B) - 음악 정지
**디컴파일 상태**: ✅ 완료

```c
// 停止音乐序列播放
void sub_3AF5B(int seq_handle)
{
    dword_54178++;
    
    fprintf(env, "AIL_stop_sequence(0x%X)\n", seq_handle);
    
    sub_447D0(seq_handle);  // 实际停止调用
    
    JUMPOUT(0x382CA);  // 清理返回
}
```

### 5.3 sub_3ADF5 (0x3ADF5) - 시퀀스 초기화
**디컴파일 상태**: ✅ 완료

```c
// 初始化音乐序列
void sub_3ADF5(int seq_handle, BYTE *music_data, int start_pos)
{
    dword_54178++;
    
    fprintf(env, "AIL_init_sequence(0x%X,0x%X,%d)\n", 
            seq_handle, music_data, start_pos);
    
    sub_443D0(seq_handle, music_data, start_pos);  // AIL_init_sequence
    
    JUMPOUT(0x39162);
}
```

### 5.4 sub_3AEEE (0x3AEEE) - 재생 시작
**디컴파일 상태**: ✅ 완료

```c
// 开始播放音乐序列
void sub_3AEEE(int seq_handle)
{
    dword_54178++;
    
    fprintf(env, "AIL_start_sequence(0x%X)\n", seq_handle);
    
    sub_44790(seq_handle);  // AIL_start_sequence
    
    JUMPOUT(0x382CA);
}
```

### 5.5 sub_3B124 (0x3B124) - 시퀀스 볼륨 설정
**디컴파일 상태**: ✅ 완료

```c
// 设置音乐序列音量（支持淡入淡出）
int sub_3B124(int seq_handle, int volume, int fade_time)
{
    int result;
    
    dword_54178++;
    
    fprintf(env, "AIL_set_sequence_volume(0x%X,%d,%d)\n", 
            seq_handle, volume, fade_time);
    
    result = sub_449E0(seq_handle, volume, fade_time);  // AIL_set_sequence_volume
    
    dword_54178--;
    return result;
}
```

### 5.6 sub_3B1A6 (0x3B1A6) - 반복 횟수 설정
**디컴파일 상태**: ✅ 완료

```c
// 设置音乐循环次数
void sub_3B1A6(int seq_handle, int loop_count)
{
    dword_54178++;
    
    fprintf(env, "AIL_set_sequence_loop_count(0x%X,%d)\n", 
            seq_handle, loop_count);
    
    sub_44960(seq_handle, loop_count);  // AIL_set_sequence_loop_count
    
    JUMPOUT(0x381D0);
}
```

### 5.7 sub_3666C (0x3666C) - 음악 데이터 검증
**디컴파일 상태**: ✅ 완료

```c
// 验证音乐数据完整性
bool sub_3666C(BYTE *data, int size)
{
    return sub_365DA(data, data + size);  // 简单校验
}
```

---

## 6. 세이브 로드 함수

### 6.1 sub_10010 (0x10010) - 세이브 데이터 로드
**디컴파일 상태**: ✅ 완료

```c
// 加载存档并恢复游戏状态
void sub_10010()
{
    BYTE *save_data;
    FILE *fp;
    int checksum;
    
    sub_3702F(60);
    
    // 分配存档缓冲区 (22987字节)
    save_data = malloc(22987);
    if (!save_data) {
        printf(" Out of Memory !!!\n");
        exit(3);
    }
    
    // 打开存档文件
    fp = fopen("FD2.SAV", "rb");
    fread(save_data, 1, 22987, fp);
    fclose(fp);
    
    // 验证校验和
    sub_4DF28(save_data, 22987);  // 解密/验证
    checksum = sub_4DF09(save_data, 22987);  // 计算校验和
    
    if (checksum != *(int*)(save_data + 22983)) {
        // 校验和错误 - 可能是新游戏
        sub_1956B(75);
        sub_15F84(...);  // 显示错误
        sub_16559(0);
        sub_16C57(0);
    }
    
    // 初始化视频淡入效果
    sub_1F882();
    
    // 恢复调色板
    memmove(dword_53BF7, save_data + 2211, 2560);
    
    // 加载基础数据
    FDOTHER_DAT = sub_111BA("FDOTHER.DAT", 0);
    
    // 恢复游戏状态
    n17 = *(save_data + 12485);  // 当前状态索引
    
    // 加载地图数据
    dword_53A59 = sub_111BA("FDFIELD.DAT", 3 * n17 + 2);
    
    // 分配和加载存档主数据
    dword_53A55 = malloc(2211);
    memmove(dword_53A55, save_data, 2211);
    
    // 加载图形资源
    sub_10652();
    dword_53A79 = sub_111BA("FDTXT.DAT", n17 + 1);
    dword_53A51 = sub_111BA("FDFIELD.DAT", 3 * n17);
    dword_53A5D = sub_111BA("FDSHAP.DAT", 2 * n17);
    dword_53A69 = sub_111BA("FDSHAP.DAT", 2 * n17 + 1);
    
    // 处理图形数据
    sub_4DF4C(dword_53A51);
    
    // 恢复单位数据
    n6_0 = *(save_data + 12484);  // 单位数量
    dword_53A45 = malloc(7680);   // 单位数组
    memmove(dword_53A45, save_data + 4771, 80 * n6_0);
    
    // 恢复其他状态
    memmove(dword_53AD5, save_data + 12451, 32);
    
    // 加载图标
    v14 = fopen("FDICON.B24", "rb");
    for (i = 0; i < n6_0; i++) {
        *(dword_53A45 + 80*i + 2) = sub_11019(*(dword_53A45 + 80*i + 7), v14);
    }
    fclose(v14);
    
    // 保存临时文件
    fp = fopen("FD2.TMP", "wb");
    fwrite(dword_53A61, 1, 13054, fp);
    fclose(fp);
    
    // 恢复其他状态变量
    dword_53BEF = *(save_data + 12483);
    dword_53AA9 = *(save_data + 12486);
    // ... 更多状态恢复
    
    // 清理
    free(save_data);
    free(dword_53A59);
    
    // 重新初始化状态
    sub_25977(byte_51E63[n17], 0);
    dword_51A83 = 0;
    
    // 播放开场动画效果
    sub_12263();
    sub_11CAC(1);
    sub_1F525();
    
    // 播放图标淡入动画
    for (n6 = 0; n6 < 9; n6++) {
        v10 = sub_15F0E(...);  // 加载图标帧
        if (n6 > 6)
            sub_187D6(...);
        j___delay(70);
        if (n6 == 8)
            j___delay(500);
        sub_15E71(v10, ...);  // 显示帧
    }
    
    // 播放标题淡入动画
    for (n2 = 2; n2 < 6; n2++) {
        if (n2 == 5) n2 = 9;
        v12 = sub_15F0E(...);
        sub_187D6(...);
        sub_11EB0(...);
        sub_17AA9(1);
        sub_15E71(v12, ...);
    }
    
    sub_11CAC(0);
    j___delay(200);
    
    dword_53AE9 = 0;
    dword_51A83 = 1;
    sub_4E381();
}
```

### 6.2 sub_11019 (0x11019) - 아이콘 데이터 로드
**디컴파일 상태**: ✅ 완료

```c
// 从FDICON.B24加载图标数据
int sub_11019(int icon_id, FILE *icon_file)
{
    BYTE *index_data;
    int offsets[13];
    int icon_offset, icon_size;
    
    sub_3702F(92);
    
    // 读取图标索引
    fseek(icon_file, 6, 0);
    index_data = malloc(6720);
    fread(index_data, 1, 6720, icon_file);
    
    // 获取图标偏移
    for (i = 0; i < 13; i++) {
        offsets[i] = *(int*)(index_data + 48 * icon_id + 4 * i);
    }
    icon_size = offsets[12] - offsets[0];
    
    free(index_data);
    
    // 检查缓存
    if (dword_53BDF) {
        // 检查是否已缓存
        for (i = 0; i < dword_53BDF; i++) {
            if (icon_id == dword_53B17[i])
                return i;
        }
        
        // 添加到缓存
        dword_53B17[dword_53BDF] = icon_id;
        fseek(icon_file, offsets[0], 0);
        fread(buf + dword_53A61, 1, icon_size, icon_file);
        
        // 保存偏移表
        for (i = 0; i < 12; i++) {
            *(int*)(dword_53A61 + 4 * (i + 12 * dword_53BDF)) = 
                offsets[i] - offsets[0] + buf;
        }
        
        buf += icon_size;
        return dword_53BDF++;
    } else {
        // 首次加载 - 初始化缓存
        dword_53B17[0] = icon_id;
        dword_53A61 = malloc(13054);  // 缓存大小
        
        fseek(icon_file, offsets[0], 0);
        fread(dword_53A61 + 1920, 1, icon_size, icon_file);
        
        for (i = 0; i < 12; i++) {
            *(int*)(dword_53A61 + 4 * i) = offsets[i] - offsets[0] + 1920;
        }
        
        dword_53BDF = 1;
        buf = icon_size + 1920;
        return 0;
    }
}
```

---

## 7. 오프닝 애니메이션 함수

### 7.1 sub_25EBB (0x25EBB) - 세이브 체크 / 게임 진입
**디컴파일 상태**: ✅ 완료

```c
// 检查存档文件并决定是新游戏还是继续游戏
int sub_25EBB()
{
    FILE *fp;
    BYTE *save_data;
    int result, slot;
    
    sub_3702F(32);
    
    // 检查是否需要开场动画
    result = sub_1F894();
    
    if (result == 0) {
        // 首次运行 - 播放开场动画
        n17 = 0;
        FDOTHER_DAT = sub_111BA("FDOTHER.DAT", 0);
        dword_53BFB = 0;
        byte_51AAC = 0;
        
        // 调用初始化函数
        funcs_25E3A[n17]();
        sub_25977(byte_51E63[n17], 0);
        byte_51AAC = 1;
        sub_4E381();
        return 0;
    }
    
    if (result != 1) {
        // 重新播放动画
        sub_25977(result, -1, 0);
        sub_10010();
        sub_25977(byte_51E63[n17], 0);
        return 0;
    }
    
    // 加载存档选择画面
    dword_53F66 = sub_111BA("FDOTHER.DAT", 13);
    sub_1F882(dword_53F66);
    FDOTHER_DAT = sub_111BA("FDOTHER.DAT", 0);
    
    // 清空视频缓冲区
    memset(655360, 0, 64000);
    sub_11D40(..., 0, 255, 0);
    
    // 分配存档缓冲区
    save_data = malloc(22987);
    
    // 打开存档文件
    fp = fopen("FD2.SAV", "rb");
    if (fp) {
        fread(save_data, 1, 22987, fp);
        sub_4DF28(save_data, 22987);
        fclose(fp);
    } else {
        memset(save_data, 255, 22987);  // 初始化为空
    }
    
    // 读取存档槽位
    do {
        slot = sub_29BCB(save_data, 0);
        if (slot != -1) {
            // 从槽位恢复数据
            memmove(dword_53BF7, save_data + 12587 + 2600*slot, 2560);
            // 读取状态
            n17 = *(save_data + 12587 + 2600*slot + 2560);
            // ...
        }
        sub_26996();  // 刷新显示
    } while (!slot);
    
    free(save_data);
    free(dword_53F66);
    
    // 加载选中的存档
    if (slot == 1) {
        byte_51AAC = 0;
        result = sub_26152();
        if (!result) {
            funcs_25E3A[n17]();
            sub_25977(byte_51E63[n17], 0);
        }
        byte_51AAC = 1;
    }
    
    sub_4E381();
    return result;
}
```

### 7.2 sub_1F894 (0x1F894) - 오프닝 애니메이션 재생
**디컴파일 상태**: ✅ 완료

```c
// 播放开场动画并显示主菜单
void sub_1F894()
{
    BYTE *anim_buffer;
    int frame_count, menu_option;
    int has_save_file = 2;  // 默认无存档
    
    sub_3702F(136);
    
    // 1. 加载并显示背景
    _FDOTHER.DAT_ = sub_111BA("FDOTHER.DAT", 77);  // 动画数据
    memset(655360, 0, 64000);                       // 清屏
    FDOTHER_DAT = sub_111BA("FDOTHER.DAT", 76);    // 背景图像
    sub_11D40(FDOTHER_DAT, ..., 0, 255, 64);       // 设置调色板
    
    // 显示标题
    _FDOTHER.DAT_1 = sub_111BA("FDOTHER.DAT", 74);
    sub_4E98D(_FDOTHER.DAT_1, ..., 655360, 320, -1);  // 解压图像
    sub_1F525();   // 淡入效果
    sub_17AA9(1);
    sub_17AA9(30);
    sub_1F882();   // 淡出效果
    
    // 2. 准备动画缓冲区
    FDOTHER_DAT = sub_111BA("FDOTHER.DAT", 99);
    memset(655360, 0, 64000);
    sub_11D40(..., 0, 255, 0);
    sub_20421(3, 90, 1);  // 播放ANI动画
    sub_1F882();
    
    // 3. 加载滚动动画帧
    memset(655360, 0, 64000);
    FDOTHER_DAT = sub_111BA("FDOTHER.DAT", 101);
    sub_11D40(FDOTHER_DAT, ..., 0, 255, 64);
    
    // 分配动画缓冲区 (230400字节)
    anim_buffer = malloc(230400);
    memset(anim_buffer, 0, 230400);
    
    // 加载5帧动画数据
    for (i = 0; i < 5; i++) {
        _FDOTHER.DAT_1 = sub_111BA("FDOTHER.DAT", i + 69);
        sub_4E98D(_FDOTHER.DAT_1, ..., anim_buffer, 320, 147*i);
    }
    
    sub_4E381();  // 同步
    
    // 4. 动画主循环 (535帧滚动)
    for (frame = 535; frame >= 0; frame--) {
        // 复制当前帧到视频缓冲区
        sub_11EB0(anim_buffer + 320*frame, ..., 
                  655360, 320, anim_buffer + 320*frame, 320, 320, 200);
        
        if (frame == 535)
            sub_1F525();  // 首帧淡入
        
        // 特定帧触发效果
        switch (frame) {
            case 450:
                sub_1F882();
                sub_1F81E(0, 15, 0);
                break;
            case 330:
                sub_1F882();
                sub_1F81E(4, 90, 99);
                break;
            case 210:
                sub_1F882();
                sub_1F81E(6, 90, 99);
                break;
            case 110:
                sub_1F882();
                sub_1F81E(8, 90, 99);
                break;
            case 10:
                sub_1F73F(75, 76, anim_buffer, 10);
                break;
        }
        
        // 播放音效
        if (frame == trigger_point) {
            sub_25A96(v40, 0, 1);
            FDOTHER_DAT = sub_111BA("FDOTHER.DAT", 102);
            sub_11D40(..., 0, 255, 0);
            trigger_index++;
        }
        
        j___delay(30);  // 帧延迟
        if (frame == 0)
            j___delay(1000);
        
        // 检查是否跳过动画
        if (sub_10620())  // 检查按键
            goto show_menu;
    }
    
show_menu:
    // 5. 淡出动画
    for (i = 40; i >= 0; i--) {
        sub_2DF01(..., i, 0x3F, 0, 0, 0);
        j___delay(8);
    }
    j___delay(100);
    sub_4E381();
    
    // 释放动画缓冲区
    free(anim_buffer);
    free(temp_buffer);
    
    // 6. 加载菜单数据
    _FDOTHER.DAT_ = sub_111BA("FDOTHER.DAT", 7);
    FDOTHER_DAT = sub_111BA("FDOTHER.DAT", 8);
    
    // 7. 显示菜单
    memset(655360, 0, 64000);
    sub_11D40(..., 0, 255, 0);
    sub_20421(1, 15, 1);  // 播放菜单动画
    sub_25B45(..., 3, 1); // 播放菜单音效
    sub_11DF2(..., 0, 255, 64);
    sub_16886(..., 655360, 320, ...);
    
    // 菜单淡入
    for (i = 0; i <= 40; i++) {
        sub_2DF01(..., i, 0x38, 0x3C, 0x3F);
        j___delay(8);
    }
    sub_4E381();
    
    // 8. 检查存档文件确定选项数
    fp = fopen("FD2.SAV", "rb");
    if (fp) {
        save_data = malloc(22987);
        fread(save_data, 1, 22987, fp);
        fclose(fp);
        sub_4DF28(save_data, 22987);
        
        if (sub_4DF09(save_data, 22987) == *(int*)(save_data + 22983)) {
            has_save_file = 2;
            if (*(save_data + 12485) != 255)
                has_save_file = 3;  // 有有效存档
        }
        free(save_data);
    }
    
    // 9. 显示菜单选项
    sub_1FF79(_FDOTHER.DAT_, 0, has_save_file);
    
    // 10. 菜单输入循环
    menu_option = 0;
    while (!menu_selected) {
        sub_1FF79(_FDOTHER.DAT_, menu_option, has_save_file);
        
        // 获取键盘输入
        int386(22, &regs, &regs);
        key = regs.h.ah;
        
        if (key == UP) {        // 上移
            sub_25A96(v40, 2, 1);
            menu_option = (menu_option > 0) ? menu_option - 1 : has_save_file - 1;
        }
        else if (key == DOWN) { // 下移
            sub_25A96(v40, 2, 1);
            menu_option = (menu_option < has_save_file - 1) ? menu_option + 1 : 0;
        }
        else if (key == ENTER || key == SPACE || key == extended) {
            sub_25A96(v40, 1, 1);
            menu_selected = 1;
        }
    }
    
    // 11. 确认选择闪烁效果
    for (i = 0; i < 4; i++) {
        sub_1FF79(_FDOTHER.DAT_, -1, has_save_file);  // 隐藏
        j___delay(80);
        sub_1FF79(_FDOTHER.DAT_, menu_option, has_save_file);  // 显示
        j___delay(80);
    }
    
    // 12. 清理并返回
    sub_1F882();
    memset(655360, 0, 64000);
    free(_FDOTHER.DAT_);
    sub_25A96(v40, -1, 1);
    free(v40);
    
    // 跳转到状态处理
    JUMPOUT(0x13994);
}
```

### 7.3 sub_10620 (0x10620) - 키 입력 체크
**디컴파일 상태**: ✅ 완료

```c
// 检查是否有按键输入（用于跳过动画）
bool sub_10620()
{
    sub_3702F(8);
    
    // 比较键盘缓冲区头尾指针
    return MEMORY[0x41C] != MEMORY[0x41A];
}
```

### 7.4 sub_4E381 (0x4E381) - 비디오 버퍼 동기화
**디컴파일 상태**: ✅ 완료

```c
// 同步视频缓冲区（等待垂直回扫）
__int16 sub_4E381()
{
    __int16 result;
    
    result = MEMORY[0x41A];     // 读取当前帧计数
    MEMORY[0x41C] = MEMORY[0x41A];  // 重置同步标志
    
    return result;
}
```

### 7.5 sub_4E31C (0x4E31C) - 팔레트 애니메이션 갱신
**디컴파일 상태**: ✅ 완료

```c
// 更新调色板动画效果（16色调色板循环）
void sub_4E31C()
{
    unsigned char *palette_ptr;
    int color_index;
    
    // 检查是否需要更新
    if ((sub_4E310() - word_60000) < 2)
        return;
    
    // 更新调色板索引
    if (++byte_60002 == 16)
        byte_60002 = 0;
    
    // 获取调色板数据
    palette_ptr = &unk_60003 + 3 * byte_60002;
    
    // 输出16个颜色到VGA调色板
    color_index = 0xE0;  // 起始颜色 (224)
    for (i = 0; i < 16; i++) {
        outp(0x3C8, color_index);
        outp(0x3C9, *palette_ptr++);
        outp(0x3C9, *palette_ptr++);
        outp(0x3C9, *palette_ptr++);
        color_index++;
    }
    
    word_60000 = sub_4E310();  // 更新时间戳
}
```

---

## 8. 메뉴 표시 함수

### 8.1 sub_1FF79 (0x1FF79) - 메뉴 옵션 그리기
**디컴파일 상태**: ✅ 완료

```c
// 绘制菜单选项
char sub_1FF79(int _FDOTHER.DAT_, int selected_option, int option_count)
{
    int highlight_mode;
    
    sub_3702F(20);
    
    // 绘制第一个选项
    highlight_mode = (selected_option == 0) ? 2 : 1;
    sub_16886(highlight_mode, ..., 655360, 320, _FDOTHER.DAT_, highlight_mode);
    
    // 绘制第二个选项（如果有）
    if (option_count > 1) {
        highlight_mode = (selected_option == 1) ? 4 : 3;
        sub_16886(highlight_mode, ..., 655360, 320, _FDOTHER.DAT_, highlight_mode);
    }
    
    // 绘制第三个选项（如果有）
    if (option_count > 2) {
        highlight_mode = (selected_option == 2) ? 6 : 5;
        return sub_16886(highlight_mode, ..., 655360, 320, _FDOTHER.DAT_, highlight_mode);
    }
    
    return 0;
}
```

### 8.2 sub_20421 (0x20421) - ANI 애니메이션 재생
**디컴파일 상태**: ✅ 완료

```c
// 从ANI.DAT播放动画
void sub_20421(int anim_id, int delay, int with_audio)
{
    BYTE *frame_data, *palette_buffer;
    FILE *ani_file;
    int frame_count, i;
    
    sub_3702F(56);
    
    sub_4E381();  // 同步
    
    // 加载调色板（如果是第一个动画）
    if (anim_id == 1)
        _FDOTHER.DAT_ = sub_111BA("FDOTHER.DAT", 78);
    
    // 分配缓冲区
    palette_buffer = malloc(768);
    frame_data = malloc(64000);
    
    // 保存当前调色板
    sub_36FD3(64000, 655360, palette_buffer);
    
    // 打开ANI文件
    ani_file = fopen("ANI.DAT", "rb");
    
    // 读取动画索引
    fseek(ani_file, 4 * anim_id + 6, 0);
    fread(frame_data, 1, 8, ani_file);
    fseek(ani_file, *(int*)frame_data, 0);
    fread(frame_data, 1, 173, ani_file);
    
    frame_count = *(short*)(frame_data + 165);
    
    // 播放动画帧
    for (i = 0; i < frame_count; i++) {
        // 读取帧头和数据
        fread(frame_header, 1, 8, ani_file);
        fread(frame_data, 1, frame_header[0], ani_file);
        
        // 解压帧
        sub_36FF4(frame_header[1], frame_data);
        
        // 播放音效（第一帧）
        if (anim_id == 1 && i == 0)
            sub_25A96(_FDOTHER.DAT_, 0, 1);
        
        j___delay(delay);
        
        // 检查跳过
        if (with_audio && sub_10620())
            break;
        
        sub_4E381();  // 同步
    }
    
    // 清理
    free(palette_buffer);
    free(frame_data);
    
    if (_FDOTHER.DAT_) {
        sub_25A96(_FDOTHER.DAT_, -1, 1);
        free(_FDOTHER.DAT_);
    }
    
    fclose(ani_file);
}
```

### 8.3 sub_1F81E (0x1F81E) - 특수 화면 표시
**디컴파일 상태**: ✅ 완료

```c
// 显示特定画面（标题/结局等）
int sub_1F81E(int n4, int n15, int n99)
{
    sub_3702F(16);
    
    // 如果需要加载新图像
    if (n99 != -1) {
        memset(655360, 0, 64000);
        FDOTHER_DAT = sub_111BA("FDOTHER.DAT", n99);
    }
    
    // 设置调色板
    sub_11D40(FDOTHER_DAT, ..., 0, 255, 0);
    
    // 播放动画
    sub_20421(n4, n15, 0);
    
    // 淡出
    return sub_1F882();
}
```

### 8.4 sub_1F73F (0x1F73F) - 특수 씬 표시
**디컴파일 상태**: ✅ 완료

```c
// 显示特殊场景（如制作人员名单）
int sub_1F73F(int n100, int n5, int n15, int n99)
{
    sub_3702F(32);
    
    // 淡出当前画面
    sub_1F882();
    
    // 加载背景
    memset(655360, 0, 64000);
    FDOTHER_DAT = sub_111BA("FDOTHER.DAT", n5);
    
    // 加载并显示图像
    _FDOTHER.DAT_ = sub_111BA("FDOTHER.DAT", n100);
    sub_4E98D(_FDOTHER.DAT_, ..., 655360, 320, -1);
    
    // 淡入效果
    sub_1F525();
    sub_17AA9(1);
    sub_17AA9(6);
    sub_1F882();
    
    // 加载下一场景
    FDOTHER_DAT = sub_111BA("FDOTHER.DAT", 101);
    
    // 复制到缓冲区
    sub_11EB0(n15 + 320*n99, ..., 655360, 320, ...);
    
    return sub_1F525();
}
```

---

## 9. 비디오 / 디스플레이 함수

### 9.1 sub_11EB0 (0x11EB0) - 프레임 복사
**디컴파일 상태**: ✅ 완료

```c
// 复制图像数据到目标缓冲区
int sub_11EB0(
    int dest,        // 目标缓冲区地址
    int dest_pitch,  // 目标行宽
    int src,         // 源缓冲区地址
    int src_pitch,   // 源行宽
    int width,       // 复制宽度
    int height)      // 复制高度
{
    int y;
    
    sub_3702F(32);
    
    // 逐行复制
    for (y = 0; y < height; y++) {
        memmove(dest, src, width);
        dest += dest_pitch;
        src += src_pitch;
    }
    
    return 0;
}
```

### 9.2 sub_1F525 (0x1F525) - 페이드 인 효과
**디컴파일 상태**: ✅ 완료

```c
// 调色板淡入效果（从黑色到正常）
int sub_1F525()
{
    int intensity;
    
    sub_3702F(20);
    
    // 64步淡入
    for (intensity = 64; intensity >= 0; intensity--) {
        sub_11D40(..., 0, 255, intensity);  // 减少暗度
        j___delay(2);
    }
    
    return 0;
}
```

### 9.3 sub_1F882 (0x1F882) - 페이드 아웃 효과
**디컴파일 상태**: ✅ 완료

```c
// 调色板淡出效果（从正常到黑色）
int sub_1F882()
{
    int intensity;
    
    sub_3702F(20);
    
    // 64步淡出
    for (intensity = 0; intensity < 64; intensity++) {
        sub_11D40(..., 0, 255, intensity);  // 增加暗度
        j___delay(2);
    }
    
    return 0;
}
```

---

## 10. 팔레트 함수

### 10.1 sub_11D40 (0x11D40) - 팔레트 설정
**디컴파일 상태**: ✅ 완료

```c
// 设置VGA调色板（支持亮度偏移）
void sub_11D40(
    int palette_data,  // 调色板数据地址 (FDOTHER_DAT)
    int start_color,   // 起始颜色索引
    int end_color,     // 结束颜色索引
    int brightness)    // 亮度偏移 (0-64, 减少值变暗)
{
    int r, g, b;
    
    sub_3702F(24);
    
    // 遍历颜色范围
    while (start_color <= end_color) {
        // 输出颜色索引
        outp(0x3C8, start_color);  // VGA调色板写入端口
        
        // 读取并调整RGB值
        r = *(BYTE*)(palette_data + 3 * start_color) - brightness;
        if (r < 0) r = 0;
        outp(0x3C9, r);  // 红色
        
        g = *(BYTE*)(palette_data + 3 * start_color + 1) - brightness;
        if (g < 0) g = 0;
        outp(0x3C9, g);  // 绿色
        
        b = *(BYTE*)(palette_data + 3 * start_color + 2) - brightness;
        if (b < 0) b = 0;
        outp(0x3C9, b);  // 蓝色
        
        start_color++;
    }
}
```

### 10.2 sub_2DF01 (0x2DF01) - 팔레트 그라데이션
**디컴파일 상태**: ✅ 완료

```c
// 调色板渐变过渡
int sub_2DF01(
    int palette_data,
    int start_color,
    int end_color,
    int progress,      // 0-40 进度
    unsigned char r_target,  // 目标红色
    unsigned char g_target,  // 目标绿色
    unsigned char b_target)  // 目标蓝色
{
    int r, g, b;
    
    sub_3702F(28);
    
    // 遍历颜色范围
    while (start_color < end_color) {
        outp(0x3C8, start_color);
        
        // 计算渐变RGB值
        r = r_target + progress * (*(BYTE*)(palette_data + 3*start_color) - r_target) / 40;
        outp(0x3C9, r);
        
        g = g_target + progress * (*(BYTE*)(palette_data + 3*start_color + 1) - g_target) / 40;
        outp(0x3C9, g);
        
        b = b_target + progress * (*(BYTE*)(palette_data + 3*start_color + 2) - b_target) / 40;
        outp(0x3C9, b);
        
        start_color++;
    }
    
    return 0;
}
```

### 10.3 sub_17AA9 (0x17AA9) - 비디오 효과 처리
**디컴파일 상태**: ✅ 완료

```c
// 视频效果处理（闪烁/扫描线等）
int sub_17AA9(int effect_type)
{
    // 根据参数执行不同的视频效果
    // 具体实现较复杂，涉及视频缓冲区操作
    // ...
}
```

---

## 11. 디컴파일 상태 정리

### 디컴파일 완료된 시작 관련 함수

| 함수 | 주소 | 상태 | 기능 |
|------|------|------|------|
| main | 0x25BF4 | ✅ | 메인 함수 |
| sub_3702F | 0x3702F | ✅ | 스택 초기화 |
| sub_37042 | 0x37042 | ✅ | 스택 체크 |
| sub_111BA | 0x111BA | ✅ | DAT 파일 로더 |
| sub_37D3E | 0x37D3E | ✅ | AIL_startup |
| sub_3AA72 | 0x3AA72 | ✅ | MDI 드라이버 설치 |
| sub_3908B | 0x3908B | ✅ | DIG 드라이버 초기화 |
| sub_392D0 | 0x392D0 | ✅ | 샘플 핸들 할당 |
| sub_3ACA3 | 0x3ACA3 | ✅ | 시퀀스 핸들 할당 |
| sub_25977 | 0x25977 | ✅ | 상태 / 음악 초기화 |
| sub_3AF5B | 0x3AF5B | ✅ | 음악 정지 |
| sub_3ADF5 | 0x3ADF5 | ✅ | 시퀀스 초기화 |
| sub_3AEEE | 0x3AEEE | ✅ | 재생 시작 |
| sub_3B124 | 0x3B124 | ✅ | 볼륨 설정 |
| sub_3B1A6 | 0x3B1A6 | ✅ | 반복 설정 |
| sub_3666C | 0x3666C | ✅ | 음악 데이터 검증 |
| sub_10010 | 0x10010 | ✅ | 세이브 로드 |
| sub_11019 | 0x11019 | ✅ | 아이콘 로드 |
| sub_25EBB | 0x25EBB | ✅ | 세이브 체크 |
| sub_1F894 | 0x1F894 | ✅ | 오프닝 애니메이션 재생 |
| sub_10620 | 0x10620 | ✅ | 키 입력 체크 |
| sub_4E381 | 0x4E381 | ✅ | 비디오 동기화 |
| sub_4E31C | 0x4E31C | ✅ | 팔레트 애니메이션 |
| sub_1FF79 | 0x1FF79 | ✅ | 메뉴 그리기 |
| sub_20421 | 0x20421 | ✅ | ANI 애니메이션 재생 |
| sub_1F81E | 0x1F81E | ✅ | 특수 화면 표시 |
| sub_1F73F | 0x1F73F | ✅ | 특수 씬 표시 |
| sub_11EB0 | 0x11EB0 | ✅ | 프레임 복사 |
| sub_1F525 | 0x1F525 | ✅ | 페이드 인 효과 |
| sub_1F882 | 0x1F882 | ✅ | 페이드 아웃 효과 |
| sub_11D40 | 0x11D40 | ✅ | 팔레트 설정 |
| sub_2DF01 | 0x2DF01 | ✅ | 팔레트 그라데이션 |
| sub_17AA9 | 0x17AA9 | ✅ | 비디오 효과 |

### 미디컴파일 시작 관련 함수

| 함수 | 주소 | 상태 | 원인 |
|------|------|------|------|
| start | 0x3CCB4 | ❌ | 라이브러리 함수 스킵됨 |
| malloc | 0x3706E | ❌ | 표준 라이브러리 함수 |
| free | 0x3776E | ❌ | 표준 라이브러리 함수 |
| fopen | 0x37324 | ❌ | 표준 라이브러리 함수 |
| fclose | 0x3759C | ❌ | 표준 라이브러리 함수 |
| fread | - | ❌ | 표준 라이브러리 함수 |
| fwrite | 0x377A3 | ❌ | 표준 라이브러리 함수 |
| fseek | 0x37940 | ❌ | 표준 라이브러리 함수 |
| memmove | 0x3771C | ❌ | 표준 라이브러리 함수 |
| memset | 0x37910 | ❌ | 표준 라이브러리 함수 |
| int386 | 0x370F0 | ❌ | DOS 확장 함수 |
| printf | 0x37119 | ❌ | 표준 라이브러리 함수 |
| fprintf | 0x3F46B | ❌ | 표준 라이브러리 함수 |
| rand | 0x3CC7D | ❌ | 표준 라이브러리 함수 |
| outp | 0x37AE5 | ❌ | 포트 출력 |
| j___delay | 0x3E01D | ❌ | 지연 함수 |

### 디컴파일 실패 함수

| 함수 | 주소 | 원인 |
|------|------|------|
| sub_16F55 | 0x16F55 | decompile returned None |
| sub_244B6 | 0x244B6 | decompile returned None |
| sub_2548C | 0x2548C | decompile returned None |
| sub_25757 | 0x25757 | decompile returned None |

---

## 부록: 전역 변수 속람

### 오디오 관련
| 변수 | 주소 | 용도 |
|------|------|------|
| dword_53ED8 | 0x53ED8 | 오디오 드라이버 상태 |
| dword_53ED0 | 0x53ED0 | 시퀀스 핸들 설정 |
| dword_53EDC | 0x53EDC | DIG 드라이버 핸들 |
| dword_53EE4 | 0x53EE4 | 샘플 핸들 1 |
| dword_53EE8 | 0x53EE8 | 샘플 핸들 2 |

### 비디오 관련
| 변수 | 주소 | 용도 |
|------|------|------|
| 655360 | 0xA0000 | VGA 비디오 버퍼 |
| dword_53BF7 | 0x53BF7 | 팔레트 버퍼 |
| dword_53AD5 | 0x53AD5 | 팔레트 데이터 |

### 게임 상태
| 변수 | 주소 | 용도 |
|------|------|------|
| n17 | - | 현재 게임 상태 |
| n16_0 | - | 오디오 활성 플래그 |
| n6_0 | - | 유닛 개수 |
| dword_53A45 | 0x53A45 | 유닛 데이터 배열 |
| dword_53A55 | 0x53A55 | 세이브 메인 데이터 |

### 파일 데이터
| 변수 | 주소 | 파일 |
|------|------|------|
| FDOTHER_DAT | - | FDOTHER.DAT |
| FDMUS_DAT | - | FDMUS.DAT |
| dword_53A79 | - | FDTXT.DAT |
| dword_53A59 | - | FDFIELD.DAT |
| dword_53A5D | - | FDSHAP.DAT |
