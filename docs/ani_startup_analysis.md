# ANI.DAT 시작 애니메이션 분석

## 개요

본 문서는 게임 《용의기사2》에서 `ANI.DAT` 파일의 로드와 재생 메커니즘을 분석한다. 시작 과정 중의 모든 관련 함수의 호출 관계와 의사 코드 구현을 포함한다.

## 목차

1. [ANI.DAT 파일 포맷](#anidat-파일-포맷)
2. [함수 호출 관계](#함수-호출-관계)
3. [핵심 함수 분석](#핵심-함수-분석)
4. [시작 흐름 타이밍](#시작-흐름-타이밍)
5. [의사 코드 구현](#의사-코드-구현)

---

## ANI.DAT 파일 포맷

### 파일 헤더 구조
```
오프셋     크기    설명
0x00      4       리소스 0 의 오프셋
0x04      4       리소스 1 의 오프셋
0x06      4*      리소스 테이블 (리소스당 4바이트)
```

### 리소스 헤더 구조 (173바이트)
```
오프셋     크기    설명
0xA5      2       Block 개수 (word)
...
```

### Block 헤더 구조 (6바이트)
```
오프셋     크기    설명
0x00      2       Block 데이터 크기
0x02      2       명령 개수
```

### 명령 타입 (0-9)

| 명령 | 함수 | 설명 |
|------|------|------|
| 0 | do_h0 | 팔레트를 단일 색상으로 설정 (96항) |
| 1 | do_h1 | 팔레트 직접 복사 (768바이트) |
| 2 | do_h2 | RLE 압축 팔레트 압축 해제 |
| 3 | do_h3 | 팔레트 부분 업데이트 |
| 4 | do_h4 | 화면을 단일 색상으로 채우기 |
| 5 | do_h5 | 화면 직접 복사 (64000바이트) |
| 6 | do_h6 | RLE 압축 화면 압축 해제 |
| 7 | do_h7 | 화면 영역 채우기 |
| 8 | do_h8 | 단일 픽셀 설정 |
| 9 | do_h9 | 화면 데이터 블록 복사 |

---

## 함수 호출 관계

### ANI.DAT 재생 핵심 호출 체인

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
│   │       ├── do_h0()  - 팔레트를 단일 색상으로
│   │       ├── do_h1()  - 팔레트 복사
│   │       ├── do_h2()  - RLE 압축 팔레트
│   │       ├── do_h3()  - 팔레트 부분 업데이트
│   │       ├── do_h4()  - 화면을 단일 색상으로 채움
│   │       ├── do_h5()  - 화면 복사
│   │       ├── do_h6()  - RLE 압축 화면
│   │       ├── do_h7()  - 화면 영역 채우기
│   │       ├── do_h8()  - 단일 픽셀 설정
│   │       └── do_h9()  - 화면 데이터 블록 복사
│   ├── delay_ms(frame_delay)
│   └── check_key_input()
└── fclose()
```

### 원본 IDA 역공학 함수 대응 관계

| C 구현 함수 | IDA 함수 주소 | 설명 |
|----------|------------|------|
| play_ani_resource() | sub_20421 | 메인 재생 함수 |
| decode_ani_block() | sub_36FF4 | Block 디코드 디스패처 |
| do_h0() | sub_36E3D | 명령 0 처리 |
| do_h1() | sub_36E57 | 명령 1 처리 |
| do_h2() | sub_36E65 | 명령 2 처리 |
| do_h3() | sub_36EA7 | 명령 3 처리 |
| do_h4() | sub_36EE0 | 명령 4 처리 |
| do_h5() | sub_36F08 | 명령 5 처리 |
| do_h6() | sub_36F24 | 명령 6 처리 |
| do_h7() | sub_36F69 | 명령 7 처리 |
| do_h8() | sub_36F82 | 명령 8 처리 |
| do_h9() | sub_36FAC | 명령 9 처리 |

---

## 핵심 함수 분석

### 1. play_ani_resource() - 메인 재생 함수

**주소:** 0x20421 (sub_20421)

**기능:** ANI.DAT 에서 지정된 애니메이션 리소스 재생

**인자:**
- res_idx: 리소스 인덱스 (0-8)
- frame_delay: 프레임 지연 (밀리초)
- wait_key: 키 입력 대기 여부

**호출자:**
- sub_1F894 (타이틀 화면)
- sub_1F81E (메뉴 처리)
- sub_24336 (게임 시작)

### 2. decode_ani_block() - Block 디코더

**주소:** 0x36FF4 (sub_36FF4)

**기능:** Block 데이터를 파싱하고 명령 시퀀스 실행

**구현 방식:** 점프 테이블로 명령 처리 함수 디스패치

### 3. run_ani_command() - 명령 디스패처

**기능:** 명령 번호에 따라 대응 처리 함수 호출

---

## 시작 흐름 타이밍

### 단계 1: 초기화
```
main()
├── init_game_machine()
├── load_resources()
│   └── load_ani_cached(0)  // 타이틀 애니메이션 리소스 정보 로드
└── run_full_startup_sequence()
```

### 단계 2: 시작 애니메이션 재생
```
run_full_startup_sequence()
├── Phase 1: 오프닝 애니메이션 재생
│   ├── load_ani_cached(0)
│   ├── for i in range(30):
│   │   └── decode_ani_block(i)  // 프레임별 디코드
│   └── 지연 처리
│
├── Phase 2: 리소스 3 애니메이션 재생 (90ms/프레임)
│   └── play_ani_resource(3, 90, 1)
│
├── Phase 3: 바 애니메이션 (535 프레임)
│   └── bar_animation()
│
├── Phase 4: 페이드 인/아웃 효과
│   └── fade_effect()
│
└── Phase 5: 리소스 1 애니메이션 재생 (15ms/프레임)
    └── play_ani_resource(1, 15, 1)
```

### 단계 3: 타이틀 화면
```
game_loop()
├── if state == STATE_TITLE:
│   └── update_ani_frame()
│       ├── load_ani_cached(g_ani_res_idx)
│       └── decode_ani_block(frame_idx)
└── render_state()
```

---

## 의사 코드 구현

### play_ani_resource() - ANI 재생 메인 함수

```c
/**
 * ANI.DAT 의 지정 리소스 재생
 * @param res_idx 리소스 인덱스
 * @param frame_delay 프레임 간 지연 (ms)
 * @param wait_key 키 입력 대기 여부
 * @return 성공 시 0, 실패 시 -1
 */
int play_ani_resource(int res_idx, int frame_delay, int wait_key)
{
    FILE* fp = fopen("ANI.DAT", "rb");
    if (!fp) return -1;
    
    // 리소스 오프셋 테이블로 탐색
    fseek(fp, 4 * res_idx + 6, SEEK_SET);
    dword offset;
    fread(&offset, 4, 1, fp);
    
    // 리소스 헤더 읽기 (173바이트)
    fseek(fp, offset, SEEK_SET);
    byte header[173];
    fread(header, 1, 173, fp);
    
    // Block 개수 획득 (오프셋 0xA5)
    word block_count = *(word*)(header + 165);
    
    // 버퍼 할당
    byte* screen_buf = malloc(64000);
    byte* palette_buf = malloc(768);
    
    // Block 별 처리
    for (int i = 0; i < block_count; i++)
    {
        // Block 헤더 읽기 (6바이트: size + cmd_count)
        byte block_header[6];
        fread(block_header, 1, 6, fp);
        
        word size = *(word*)(block_header + 0);
        word cmd_count = *(word*)(block_header + 2);
        
        if (size == 0 || cmd_count == 0) continue;
        
        // Block 데이터 읽기
        byte* block_data = malloc(size);
        fread(block_data, 1, size, fp);
        
        // 버퍼 클리어
        memset(screen_buf, 0, 64000);
        memset(palette_buf, 0, 768);
        
        // Block 디코드
        decode_ani_block(block_data, size, cmd_count, screen_buf, palette_buf);
        
        // 화면에 렌더링
        render_frame(screen_buf);
        apply_palette(palette_buf);
        
        // 프레임 간 지연
        delay_ms(frame_delay);
        
        // 키 입력 체크
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

### decode_ani_block() - Block 디코더

```c
/**
 * ANI Block 디코드
 * @param data Block 데이터
 * @param size 데이터 크기
 * @param cmd_count 명령 개수
 * @param screen 화면 버퍼
 * @param palette 팔레트 버퍼
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

### run_ani_command() - 명령 디스패처

```c
/**
 * ANI 명령 실행
 * @param cmd 명령 번호 (0-9)
 * @param data 명령 데이터
 * @param size 남은 데이터 크기
 * @param screen 화면 버퍼
 * @param palette 팔레트 버퍼
 * @return 소비된 바이트 수
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

### do_h0() - 팔레트를 단일 색상으로 설정

```c
/**
 * 명령 0: 팔레트 앞 96항을 단일 색상으로 설정
 * @param data 1바이트 색상 값
 * @param palette 팔레트 버퍼
 * @return 소비 바이트 수 (1)
 */
int do_h0(byte* data, byte* palette)
{
    byte b = data[0];
    dword color = (b << 16) | (b << 8) | b;  // RGB 동일
    
    dword* p = (dword*)palette;
    for (int i = 0; i < 96; i++)
        p[i] = color;
    
    return 1;
}
```

### do_h1() - 팔레트 직접 복사

```c
/**
 * 명령 1: 팔레트 데이터 직접 복사
 * @param data 팔레트 데이터 (최대 768바이트)
 * @param size 데이터 크기
 * @param palette 팔레트 버퍼
 * @return 소비 바이트 수
 */
int do_h1(byte* data, int size, byte* palette)
{
    int copy_size = min(size, 768);
    memcpy(palette, data, copy_size);
    return copy_size;
}
```

### do_h2() - RLE 압축 팔레트

```c
/**
 * 명령 2: RLE 압축 팔레트 압축 해제
 * @param data RLE 압축 데이터
 * @param size 데이터 크기
 * @param palette 팔레트 버퍼
 * @return 소비 바이트 수
 */
int do_h2(byte* data, int size, byte* palette)
{
    int filled = 0;
    int pos = 0;
    
    while (filled < 768 && pos < size)
    {
        byte b = data[pos++];
        
        if ((b & 0xC0) == 0xC0)  // RLE 마커
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

### do_h3() - 팔레트 부분 업데이트

```c
/**
 * 명령 3: 팔레트 부분 업데이트
 * @param data 업데이트 데이터
 * @param size 데이터 크기
 * @param palette 팔레트 버퍼
 * @return 소비 바이트 수
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

### do_h4() - 단일 색상으로 화면 채우기

```c
/**
 * 명령 4: 단일 색상으로 화면 채우기
 * @param data 1바이트 색상 값
 * @param screen 화면 버퍼
 * @return 소비 바이트 수 (1)
 */
int do_h4(byte* data, byte* screen)
{
    byte b = data[0];
    dword color = (b << 16) | (b << 8) | b;
    
    dword* p = (dword*)screen;
    for (int i = 0; i < 10000; i++)  // 앞 40000바이트는 dword 로 채움
        p[i] = color;
    
    for (int i = 40000; i < 64000; i++)  // 나머지 바이트
        screen[i] = b;
    
    return 1;
}
```

### do_h5() - 화면 직접 복사

```c
/**
 * 명령 5: 화면 데이터 직접 복사 (64000바이트)
 * @param data 화면 데이터
 * @param screen 화면 버퍼
 * @return 소비 바이트 수 (64000)
 */
int do_h5(byte* data, byte* screen)
{
    memcpy(screen, data, 64000);
    return 64000;
}
```

### do_h6() - RLE 압축 화면

```c
/**
 * 명령 6: RLE 압축 화면 압축 해제
 * @param data RLE 압축 데이터
 * @param size 데이터 크기
 * @param screen 화면 버퍼
 * @return 소비 바이트 수
 */
int do_h6(byte* data, int size, byte* screen)
{
    int filled = 0;
    int pos = 0;
    
    while (filled < 64000 && pos < size)
    {
        byte b = data[pos++];
        
        if ((b & 0xC0) == 0xC0)  // RLE 마커
        {
            int run = b & 0x3F;
            byte value = data[pos++];
            int words = run >> 1;
            int bytes = run & 1;
            
            // word (2바이트) 채우기
            for (int i = 0; i < words && filled < 64000; i++)
            {
                screen[filled++] = value;
                if (filled < 64000) screen[filled++] = value;
            }
            
            // 단일 바이트 채우기
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

### do_h7() - 화면 영역 채우기

```c
/**
 * 명령 7: 화면 영역 채우기
 * @param data 채우기 데이터
 * @param size 데이터 크기
 * @param screen 화면 버퍼
 * @return 소비 바이트 수
 */
int do_h7(byte* data, int size, byte* screen)
{
    int pos = 0;
    
    // 2바이트 count 읽기
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

### do_h8() - 단일 픽셀 설정

```c
/**
 * 명령 8: 여러 단일 픽셀 설정
 * @param data 픽셀 데이터
 * @param size 데이터 크기
 * @param screen 화면 버퍼
 * @return 소비 바이트 수
 */
int do_h8(byte* data, int size, byte* screen)
{
    int pos = 0;
    
    // 2바이트 count 읽기
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

### do_h9() - 화면 데이터 블록 복사

```c
/**
 * 명령 9: 화면 데이터 블록 복사
 * @param data 블록 데이터
 * @param size 데이터 크기
 * @param screen 화면 버퍼
 * @return 소비 바이트 수
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

## 전역 변수

| 변수명 | 주소 | 타입 | 설명 |
|--------|------|------|------|
| ani_screen_buf | - | byte[64000] | 화면 버퍼 |
| ani_palette_buf | - | byte[768] | 팔레트 버퍼 |
| g_ani_res_idx | - | int | 현재 ANI 리소스 인덱스 |
| g_ani_frame | 0x138 | int | 현재 프레임 번호 |
| g_ani_block_count | - | int | Block 개수 |
| g_ani_blocks | - | ANIBlockInfo* | Block 정보 캐시 |
| g_ani_cache | - | byte* | Block 데이터 캐시 |

---

## 정리

ANI.DAT 애니메이션 시스템은 명령 기반 프레임 기술 포맷을 사용한다:

1. **파일 구조**: 리소스 테이블 → 리소스 헤더 → Block 헤더 → Block 데이터
2. **명령 시스템**: 10종 명령으로 팔레트와 화면 데이터 처리
3. **압축 방식**: RLE 압축과 직접 데이터 복사 지원
4. **재생 메커니즘**: 프레임별 독립 디코드, 프레임 간 지연과 키 인터럽트 지원

시작 흐름에서의 ANI 리소스 사용:
- **리소스 0**: 타이틀 애니메이션
- **리소스 1**: 메뉴 애니메이션 (15ms/프레임)
- **리소스 3**: 오프닝 애니메이션 (90ms/프레임)
