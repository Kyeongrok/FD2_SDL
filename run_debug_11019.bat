@echo off
chcp 65001 >nul
title FD2 Debug - sub_11019

cd /d "D:\testworkspace\fd2dat\bin"

echo ================================================
echo   FD2 调试器 - 断点 sub_11019
echo ================================================
echo.
echo 目标地址: 0x11019 (加载图标)
echo.
echo 按键说明:
echo   F5    - 继续运行
echo   F9    - 设置/取消断点
echo   F10   - 单步跳过
echo   F11   - 单步进入
echo   ALT+F5 - 运行到断点
echo.
echo ================================================
echo 在调试器中输入以下命令:
echo.
echo   bpcs                       ; 清空断点
echo   bp 11019                   ; 在 sub_11019 设置断点
echo   s                          ; 执行一步查看调用参数
echo   d 53A45 L 80               ; 查看第一个单位数据结构
echo.
echo ================================================
echo 按任意键启动DOSBox-X调试器...
pause >nul

start "FD2 Debug" /d "D:\testworkspace\fd2dat\dosbox-x" "dosbox-x.exe" -conf "D:\testworkspace\fd2dat\bin\debug.conf" -debug "..\game\fd2.exe"
