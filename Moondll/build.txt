@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1

REM ========================================
REM 只需修改这一行的名字！
REM ========================================
set DLL_NAME=MoonEngine
REM ========================================

echo ========================================
echo   编译目标: %DLL_NAME%.dll
echo   导入库: %DLL_NAME%.lib
echo ========================================
echo.

cd /d "%~dp0"

echo [1/3] 编译 GLAD 对象文件...
cl GLAD\glad.c /c /I"GLAD" /I"."
if %errorlevel% neq 0 goto error

echo.
echo [2/3] 从对象文件生成静态库...
lib /OUT:glad.lib glad.obj
if %errorlevel% neq 0 goto error

echo.
echo [3/3] 编译 MoonEngine 主 DLL...
REM 使用 /link /IMPLIB 强制指定 lib 名称
cl Moon_Drawing_Function.c Moon_Engine_Function.c Moon_Utility_Function.c /LD /I"." /I"GLAD" /I"KHR" /Fe:%DLL_NAME%.dll /link /IMPLIB:%DLL_NAME%.lib glad.lib SDL3.lib glfw3.lib user32.lib gdi32.lib shell32.lib winmm.lib ucrt.lib /NODEFAULTLIB:libucrt.lib

if %errorlevel% neq 0 goto error

echo.
echo 清理临时文件...
del *.obj *.exp 2>nul
del glad.dll 2>nul
del glad.lib 2>nul

echo.
echo ========================================
echo   编译成功！
echo   生成文件:
dir %DLL_NAME%.dll
dir %DLL_NAME%.lib
echo ========================================
echo.
echo 提示: 开发时需要 %DLL_NAME%.lib 和头文件
echo ========================================
pause
exit /b 0

:error
echo.
echo ========================================
echo   编译失败！错误代码: %errorlevel%
echo ========================================
pause
exit /b %errorlevel%