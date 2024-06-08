@echo off
chcp 65001

@REM xcopy ..\lib .\lib /Y /E /S /H /I /q
@REM call ..\external_lib\libcopy.bat lib build

mkdir build
xcopy "..\external_lib\glfw\lib-mingw-w64\*.dll" ".\build" /Y /E /S /q

@REM https://gen.glad.sh/
@REM https://github.com/glfw/glfw/releases
@REM https://github.com/datenwolf/linmath.h

gcc -Lbuild/ -Ilib/ -Wall -g main.c -o build/main.exe -I../external_lib/linmath/include -I../external_lib/glfw/include -I../external_lib/glad/include ../external_lib/glfw/lib-mingw-w64/libglfw3dll.a -lgdi32