# FD2重新实现Makefile
# 基于IDA Pro MCP服务器分析

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -DSDL_MAIN_HANDLED

# 目录结构
SRC_DIR = src
INC_DIR = include
LIB_DIR = lib
BIN_DIR = bin
DATA_DIR = data
THIRDPARTY_DIR = thirdparty

# SDL2支持
SDL_CFLAGS = -DUSE_SDL -I$(INC_DIR)
SDL_LDFLAGS = -L$(LIB_DIR)
SDL_LIBS = -lSDL2main -lSDL2 -lm

# 核心模块
CORE_MODULES = $(SRC_DIR)/fd2_palette.c $(SRC_DIR)/fd2_dat.c $(SRC_DIR)/fd2_rle.c $(SRC_DIR)/fd2_video.c
# 图像解码模块
CORE_MODULES += $(SRC_DIR)/fd2_image.c
# 资源加载模块
CORE_MODULES += $(SRC_DIR)/fd2_resources.c
# 移动系统模块
CORE_MODULES += $(SRC_DIR)/fd2_movement.c

# 默认目标：非SDL版本
all: fd2_game fd2_game_sdl fd2_reimpl fd2_reimpl_sdl test_unit_render test_map_render

# 非SDL版本（控制台ASCII）
fd2_game: $(SRC_DIR)/fd2_game.c $(CORE_MODULES) $(INC_DIR)/fd2_sdl_renderer.h $(INC_DIR)/fd2_types.h $(INC_DIR)/fd2_palette.h $(INC_DIR)/fd2_dat.h $(INC_DIR)/fd2_rle.h $(INC_DIR)/fd2_video.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -o $(BIN_DIR)/fd2_game $(SRC_DIR)/fd2_game.c $(CORE_MODULES) -lm

# SDL2图形版本
fd2_game_sdl: $(SRC_DIR)/fd2_game.c $(CORE_MODULES) $(SRC_DIR)/fd2_sdl_renderer_impl.c $(INC_DIR)/fd2_sdl_renderer.h $(INC_DIR)/fd2_types.h $(INC_DIR)/fd2_palette.h $(INC_DIR)/fd2_dat.h $(INC_DIR)/fd2_rle.h $(INC_DIR)/fd2_video.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -I$(INC_DIR) -o $(BIN_DIR)/fd2_game_sdl $(SRC_DIR)/fd2_game.c $(CORE_MODULES) $(SRC_DIR)/fd2_sdl_renderer_impl.c $(SDL_LDFLAGS) $(SDL_LIBS)

# fd2_reimpl SDL版本
fd2_reimpl_sdl: $(SRC_DIR)/fd2_reimpl.c $(SRC_DIR)/fd2_sdl_renderer_impl.c $(INC_DIR)/fd2_reimpl.h $(INC_DIR)/fd2_sdl_renderer.h
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -o $(BIN_DIR)/fd2_reimpl_sdl $(SRC_DIR)/fd2_reimpl.c $(SRC_DIR)/fd2_sdl_renderer_impl.c $(SDL_LDFLAGS) $(SDL_LIBS)

# fd2_reimpl 非SDL版本
fd2_reimpl: $(SRC_DIR)/fd2_reimpl.c $(INC_DIR)/fd2_reimpl.h
	$(CC) $(CFLAGS) -o $(BIN_DIR)/fd2_reimpl $(SRC_DIR)/fd2_reimpl.c -lm

# SDL图形测试
test-startup-sdl: $(SRC_DIR)/test_startup_sdl.c $(SRC_DIR)/fd2_dat.c $(SRC_DIR)/fd2_rle.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -I$(INC_DIR) -I$(INC_DIR)/SDL2 -o $(BIN_DIR)/test_startup_sdl $(SRC_DIR)/test_startup_sdl.c $(SDL_LDFLAGS) $(SDL_LIBS)

# 测试程序
test_unit: test_unit.c $(SRC_DIR)/fd2_unit.c $(INC_DIR)/fd2_unit.h $(INC_DIR)/fd2_types.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -o $(BIN_DIR)/test_unit test_unit.c $(SRC_DIR)/fd2_unit.c -lm

test_map: test_map.c $(SRC_DIR)/fd2_map.c $(SRC_DIR)/fd2_dat.c $(INC_DIR)/fd2_map.h $(INC_DIR)/fd2_dat.h $(INC_DIR)/fd2_types.h
	$(CC) $(CFLAGS) -I$(INC_DIR) -o $(BIN_DIR)/test_map test_map.c $(SRC_DIR)/fd2_map.c $(SRC_DIR)/fd2_dat.c -lm

# New: test image decoding module
test-image: src/test_image.c $(SRC_DIR)/fd2_image.c src/fd2_test_logging.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -o $(BIN_DIR)/test_image src/test_image.c $(SRC_DIR)/fd2_image.c src/fd2_test_logging.c -lm

# 单位移动测试
test_unit_render: src/test_unit_render.c $(CORE_MODULES)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -I$(INC_DIR) -o $(BIN_DIR)/test_unit_render src/test_unit_render.c $(CORE_MODULES) $(SDL_LDFLAGS) $(SDL_LIBS)

# 地图渲染测试
test_map_render: src/test_map_render.c $(CORE_MODULES)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -I$(INC_DIR) -o $(BIN_DIR)/test_map_render src/test_map_render.c $(CORE_MODULES) $(SDL_LDFLAGS) $(SDL_LIBS)

clean:
	rm -f $(BIN_DIR)/*.exe $(BIN_DIR)/*.o *.o

run: $(BIN_DIR)/fd2_game
	$(BIN_DIR)/fd2_game

run-test-unit: $(BIN_DIR)/test_unit
	$(BIN_DIR)/test_unit

run-test-map: $(BIN_DIR)/test_map
	$(BIN_DIR)/test_map

test-startup-sdl: $(SRC_DIR)/test_startup_sdl.c $(SRC_DIR)/fd2_dat.c $(SRC_DIR)/fd2_rle.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -I$(INC_DIR) -I$(INC_DIR)/SDL2 -o $(BIN_DIR)/test_startup_sdl $(SRC_DIR)/test_startup_sdl.c $(SDL_LDFLAGS) $(SDL_LIBS)

.PHONY: all clean run run-sdl test-ani test-startup-sdl run-test-sdl run-test-unit run-test-map
