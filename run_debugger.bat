@echo off
chcp 65001 >nul
title FD2 调试器

echo ================================================
echo   FD2 DOSBox-X 调试器
echo ================================================
echo.
echo 启动调试器...
echo.

set DOSBOX=D:\testworkspace\fd2dat\dosbox-x\dosbox-x.exe
set CONFIG=D:\testworkspace\fd2dat\bin\debug.conf
set GAME=..\game\fd2.exe

echo 游戏文件: %GAME%
echo 配置文件: %CONFIG%
echo.
echo 调试器快捷键:
echo   F5  = 运行/继续
echo   F9  = 设置断点
echo   F10 = 单步跳过
echo   F11 = 单步进入
echo.
echo ================================================
echo 建议断点命令 (在调试器中输入):
echo.
echo   bp 11019          ; sub_11019 加载图标
echo   bp 10010          ; sub_10010 存档加载
echo.
echo   d 53A45 L 80     ; 查看第一个单位结构
echo   d 53A45+7 B 1    ; 查看图标ID (+7偏移)
echo.
echo ================================================
echo 按任意键启动...
pause >nul

start "FD2 Debug" /d "D:\testworkspace\fd2dat\dosbox-x" "%DOSBOX%" -conf "%CONFIG%" -debug "%GAME%"
