# 용의기사2 (FD2) SDL 재구현

IDA Pro 역공학을 기반으로 한 용의기사2 (Dragon Knight 2 / 炎龙骑士团2) 의 SDL2 재구현.

## 필수 요구사항

- **MSYS2** (MinGW64 환경)
- **gcc** 15.2.0 이상
- **SDL2** (`mingw-w64-x86_64-SDL2`)
- **make** (`mingw-w64-x86_64-make`)
- **원본 FD2 게임 데이터 파일** (저작권 관계로 저장소에 포함되지 않음)

### 의존성 설치 (MSYS2)

```bash
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-make
```

## 빌드

프로젝트 루트에서:

```bash
mkdir -p bin
mingw32-make fd2_game_sdl        # SDL2 그래픽 버전 (기본)
mingw32-make fd2_game            # 콘솔 ASCII 버전
mingw32-make fd2_reimpl          # 참조 구현 (콘솔)
mingw32-make fd2_reimpl_sdl      # 참조 구현 (SDL2)
```

빌드 산출물은 `bin/` 디렉터리에 생성됩니다.

> **참고**: Makefile 의 기본 `all` 타겟은 현재 없는 `test_unit_render.c` / `test_map_render.c` 를 참조하므로 실패합니다. 개별 타겟을 지정해서 빌드하세요.

## 게임 데이터 파일 준비

재구현 바이너리는 원본 게임의 `.DAT` 파일을 **작업 디렉터리 기준**으로 열기 때문에, 프로젝트 루트에 복사해야 합니다:

```
ANI.DAT
BG.DAT
DATO.DAT
FDFIELD.DAT
FDMUS.DAT
FDOTHER.DAT
FDSHAP.DAT
FDTXT.DAT
FIGANI.DAT
```

## 실행

### PowerShell

```powershell
$env:Path = "C:\msys64\mingw64\bin;$env:Path"
.\bin\fd2_game_sdl.exe
```

### Git Bash / MSYS2 Bash

```bash
PATH="/c/msys64/mingw64/bin:$PATH" ./bin/fd2_game_sdl.exe
```

### cmd.exe

```cmd
set PATH=C:\msys64\mingw64\bin;%PATH%
bin\fd2_game_sdl.exe
```

### 시스템 PATH 에 영구 등록

Windows 환경 변수 `Path` 에 `C:\msys64\mingw64\bin` 을 추가하면 매 실행마다 PATH 를 지정할 필요가 없습니다.

## 창 사양

- 창 크기: 640×400
- 논리 해상도: 320×200 (VGA 모드 13h)
- 배율: 2x

## 문서

- `docs/FD2_Startup_Flow_Analysis.md` — 시작 흐름 분석
- `docs/FD2_GameLogic_Analysis.md` — 게임 로직 분석
- `docs/DAT文件结构分析.md` — DAT 파일 구조 분석
- `docs/ani_startup_analysis.md` — ANI.DAT 애니메이션 분석
- `README_FD2_REIMPL.md` — 재구현 개요 및 버그 수정 이력
- `RLE_FORMAT_FINAL.md` — RLE 압축 포맷 상세

## 미구현 / 알려진 한계

1. 실제 퍼즐 메커니즘 (FDFIELD / FDSHAP 데이터 활용)
2. FDMUS.DAT 음악 재생 (XMI 포맷 디코딩)
3. Makefile 의 `test_unit_render` / `test_map_render` 타겟은 소스 파일 부재로 실패
