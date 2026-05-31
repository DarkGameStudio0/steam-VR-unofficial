@echo off
REM ======================================================
REM Plague Inc. VR  —  Windows 编译脚本 (MinGW + raylib)
REM ======================================================
REM
REM 使用方式：
REM   1. 从 https://github.com/raysan5/raylib/releases
REM      下载 raylib-5.0_win64_mingw-w64.zip
REM   2. 解压到 raylib\ 目录（和 plague_vr.c 同目录）
REM      即：PlagueVR_C\raylib\include\
REM                       raylib\lib\
REM   3. 双击运行本脚本
REM
REM 如果 raylib 在其他路径，修改下面两个变量
REM ======================================================

set RAYLIB_DIR=%~dp0raylib
set SRC=%~dp0plague_vr.c
set OUT=%~dp0plague_vr.exe

echo === Plague Inc. VR Builder ===
echo.
echo 源文件: %SRC%
echo raylib:  %RAYLIB_DIR%
echo.

IF NOT EXIST "%RAYLIB_DIR%\include\raylib.h" (
    echo [错误] 找不到 raylib.h
    echo.
    echo 请先下载 raylib 5.0 MinGW 版：
    echo https://github.com/raysan5/raylib/releases
    echo.
    echo 解压到 %RAYLIB_DIR%
    pause
    exit /b 1
)

echo 正在编译...
gcc "%SRC%" -o "%OUT%" ^
    -I"%RAYLIB_DIR%\include" -L"%RAYLIB_DIR%\lib" ^
    -lraylib -lopengl32 -lgdi32 -lwinmm -lm ^
    -O2 -s

IF %ERRORLEVEL% EQU 0 (
    echo.
    echo [成功] 编译完成！
    echo 运行: %OUT%
    start "" "%OUT%"
) ELSE (
    echo.
    echo [失败] 编译出错，错误码 %ERRORLEVEL%
)

pause
