@echo off
chcp 65001

xcopy ..\lib .\lib /Y /E /S /H /I /q
call ..\external_lib\libcopy.bat lib build

gcc -Lbuild/ -Ilib/ -Wall -g main.c -o build/main.exe -lfreeglut -lopengl32 -lglu32 -lfreetype -lcomdlg32