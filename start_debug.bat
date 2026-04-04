@echo off
chcp 65001 >nul
title FD2 Debug

echo ========================================
echo   FD2 Debug Launcher
echo ========================================
echo.

cd /d "%~dp0dosbox-x"

echo 启动DOSBox-X调试器...
start "" dosbox-x.exe "..\game\fd2.exe" -conf "..\bin\debug.conf" -startdebug

echo.
echo DOSBox应该已经启动!
echo.
echo 如果没有看到窗口,请检查任务栏
echo.
echo ========================================
echo 使用说明:
echo ========================================
echo 1. 游戏启动后,按 Alt+Pause 进入调试器
echo.
echo 2. 设置断点:
echo    BP 0x37324   (fopen)
echo    BP 0x374A0   (fread)
echo.
echo 3. 运行:
echo    RUN
echo.
echo 4. 断点命中后查看:
echo    D DS:SI   (文件名)
echo    R         (寄存器)
echo ========================================
pause
