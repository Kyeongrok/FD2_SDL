@echo off
chcp 65001 >nul
title FD2 Debug Launcher

echo ================================================
echo   FD2 调试器启动
echo ================================================
echo.
echo 启动选项:
echo   1. 运行游戏到存档加载 (sub_11019)
echo   2. 断点: 0x10010 (存档加载)
echo   3. 断点: 0x25977 (状态初始化)
echo   4. 断点: 0x25EBB (检查存档)
echo   5. 退出
echo.

set /p choice=选择 (1-5):

if "%choice%"=="1" goto :run_11019
if "%choice%"=="2" goto :run_10010
if "%choice%"=="3" goto :run_25977
if "%choice%"=="4" goto :run_25EBB
if "%choice%"=="5" exit

:run_11019
echo.
echo 启动调试模式: 断点 0x11019
echo 断点命令: bp 11019
echo 当断点命中时，输入以下命令查看数据结构:
echo   d 53A45 L 80    - 查看第一个单位(80字节)
echo   d 53A45+50 L 80 - 查看第二个单位
echo.
start "FD2 Debug" /d "D:\testworkspace\fd2dat\dosbox-x" "dosbox-x.exe" -conf "D:\testworkspace\fd2dat\bin\debug.conf" -debug "..\game\fd2.exe"
goto :end

:run_10010
echo.
echo 启动调试模式: 断点 0x10010 (存档加载)
echo.
start "FD2 Debug" /d "D:\testworkspace\fd2dat\dosbox-x" "dosbox-x.exe" -conf "D:\testworkspace\fd2dat\bin\debug.conf" -debug "..\game\fd2.exe"
goto :end

:run_25977
echo.
echo 启动调试模式: 断点 0x25977 (游戏状态)
echo.
start "FD2 Debug" /d "D:\testworkspace\fd2dat\dosbox-x" "dosbox-x.exe" -conf "D:\testworkspace\fd2dat\bin\debug.conf" -debug "..\game\fd2.exe"
goto :end

:run_25EBB
echo.
echo 启动调试模式: 断点 0x25EBB (存档检查)
echo.
start "FD2 Debug" /d "D:\testworkspace\fd2dat\dosbox-x" "dosbox-x.exe" -conf "D:\testworkspace\fd2dat\bin\debug.conf" -debug "..\game\fd2.exe"
goto :end

:end
echo.
echo 按任意键退出...
pause >nul
