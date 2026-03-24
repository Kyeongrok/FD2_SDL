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

# 默认目标：非SDL版本
all: fd2_game fd2_game_sdl fd2_reimpl fd2_reimpl_sdl

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

clean:
	rm -f $(BIN_DIR)/*.exe $(BIN_DIR)/*.o *.o

run: $(BIN_DIR)/fd2_game
	$(BIN_DIR)/fd2_game

run-sdl: $(BIN_DIR)/fd2_game_sdl
	$(BIN_DIR)/fd2_game_sdl

test-ani: $(BIN_DIR)/test_ani
	$(BIN_DIR)/test_ani $(DATA_DIR)/fd2/ANI.DAT

run-test-sdl: $(BIN_DIR)/test_startup_sdl
	cd $(DATA_DIR)/fd2 && ../../$(BIN_DIR)/test_startup_sdl

.PHONY: all clean run run-sdl test-ani test-startup-sdl run-test-sdl
