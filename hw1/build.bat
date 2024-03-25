@echo off
chcp 65001
xcopy ..\lib .\lib /Y /E /S /H /I
xcopy ..\freeglut .\freeglut /Y /E /S /H /I
xcopy ..\dll .\build /Y /E /S /H /I
xcopy ..\freeglut\bin\libfreeglut.dll .\build /Y
gcc -Lfreeglut/bin/ -Ifreeglut/include/ -g main.c -o build/main.exe -lfreeglut -lopengl32 -lglu32