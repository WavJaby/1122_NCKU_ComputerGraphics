@echo off
chcp 65001

xcopy ..\lib .\lib /Y /E /S /H /I /q
call ..\external_lib\libcopy_final.bat lib build

@REM https://github.com/datenwolf/linmath.h

@REM gcc -Lbuild/ -Ilib/ -Wall -g main.c -o build/main.exe -I../external_lib/glfw/include -I../external_lib/glad/include ../external_lib/glfw/lib-mingw-w64/libglfw3dll.a -lgdi32
gcc -Lbuild/ -Ilib/ -g main.c -o build/main.exe ./lib/libglfw3dll.a -lgdi32 -lfreetype