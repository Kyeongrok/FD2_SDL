@echo off
chcp 65001 >nul
title FD2 Debug Launcher
echo ================================================
echo   FD2 DOSBox 调试器
echo ================================================
echo.
echo 启动DOSBox-X调试器...
echo.

set DOSBOX=D:\testworkspace\fd2dat\dosbox-x\dosbox-x.exe
set GAME_DIR=D:\testworkspace\fd2dat\game
set CONFIG=D:\testworkspace\fd2dat\bin/debug.conf

echo 命令: %DOSBOX% -conf %CONFIG% -startdebug
echo.

start "FD2 Debug" /d "D:\testworkspace\fd2dat\dosbox-x" "%DOSBOX%" -conf "%CONFIG%" -startdebug

echo.
echo 如果DOSBox没有启动,请手动运行:
echo %DOSBOX% -conf "%CONFIG%" -startdebug
pause
