# FD2.exe 재구현 요약

## 실행 파일
- `fd2_game_sdl.exe` (118KB) - 주요 구현, SDL2 그래픽 모드
- `fd2_game.exe` (118KB) - 콘솔 ASCII 모드
- `fd2_reimpl_sdl.exe` (171KB) - 참고 구현 (SDL2)
- `fd2_reimpl.exe` - 참고 구현 (콘솔)

## 빌드 명령
```bash
# 非SDL（控制台）
gcc -Wall -Wextra -std=c99 -g -o fd2_game fd2_game.c -lm

# SDL2 图形模式
gcc -Wall -Wextra -std=c99 -g -DSDL_MAIN_HANDLED -DUSE_SDL -Iinclude -o fd2_game_sdl fd2_game.c fd2_sdl_renderer_impl.c -Llib -lSDL2main -lSDL2 -lm
```

## ANI.DAT 형식 (IDA Pro 리버스 엔지니어링)

### 파일 구조
- Header: 6바이트 "LLLLLL" + 리소스 테이블
- 리소스 offset = 4 * resource_index + 6 from file start
- ANI.DAT 리소스 0: 173바이트 header (offset 165 = block count WORD)
- Block: [size(WORD), cmd_count(WORD), unk(WORD), unk(WORD)] + 명령 데이터

### ANI 명령 handler
dispatcher: sub_36FF4 (0x36ff4), 함수 테이블 at 0x5276a

**palette 명령 (buf[768], 앞 256바이트 = screen_buffer[0-255]):**
- h0 (0x36E3D): memset32 palette 768 bytes
- h1 (0x36E57): memcpy 768바이트를 palette로
- h2 (0x36E65): RLE 압축 해제하여 palette로
- h3 (0x36EA7): palette 패치 [count][offset*3][len][data]...

**screen 명령 (screen_buffer[64000]):**
- h4 (0x36EE0): memset32 screen 64000 bytes
- h5 (0x36F08): memcpy 64000바이트를 screen으로
- h6 (0x36F24): RLE 압축 해제하여 screen으로
- h7 (0x36F69): screen 패치 [count][offset][value]... off=256부터 시작
- h8 (0x36F82): screen memset 패치 [count][offset][stride][value]
- h9 (0x36FAC): screen memcpy 패치 [count][offset][stride][data]

### 핵심 발견
1. ANI palette는 8비트 값이며, SDL palette로 직접 복사됨 (6→8비트 변환 불필요)
2. 프레임마다 ANI palette를 적용해야 함, 그렇지 않으면 화면이 흑백이 됨
3. n655360 = 655360 = 0xA0000 (VGA VRAM 세그먼트)

## 수정된 버그
1. **present() 렌더링 잘못된 buffer** → fd2_sdl_render_frame(g_render.screen_buffer) 사용
2. **ANI palette 미적용** → 프레임마다 ani_palette_buf에서 768바이트 복사
3. **ANI h2/h6 포인터 연산** → word memset 역방향 쓰기 수정
4. **ANI h3 offset** → palette offset = off * 3
5. **ANI h7** → screen_buffer[256]부터 쓰기 시작
6. **SDLK_R/P 컴파일 오류** → 존재하지 않는 상수 삭제

## 게임 상태 머신
```
STATE_TITLE (1) - ANI 动画，Enter 进入关卡选择
STATE_LEVEL_SELECT (3) - 选择关卡 1-50
STATE_PLAYING (4) - 玩游戏
STATE_PAUSED (5) - 按 P 暂停
STATE_WIN (6) - 过关
```

## 미완료 작업
1. 게임 플로우 테스트
2. 실제 퍼즐 메커니즘 구현 (FDFIELD/FDSHAP 데이터 사용)
3. FDMUS.DAT 음악 재생 구현 (XMI 형식)
