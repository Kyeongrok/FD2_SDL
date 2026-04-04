@echo off
title FD2 Debug

cd /d "D:\testworkspace\fd2dat\dosbox-x"

start "" "dosbox-x.exe" -debug "..\game\fd2.exe" "-conf" "..\bin\debug.conf"
