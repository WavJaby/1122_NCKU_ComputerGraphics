@echo off
chcp 65001
xcopy ..\lib .\lib /Y /E /S /H /I /q
xcopy ..\freeglut\include .\lib /Y /E /S /H /I /q
xcopy ..\freetype .\lib /Y /E /S /H /I /q
@REM xcopy ..\openGLES .\lib /Y /E /S /H /I /q

xcopy ..\bin\*.dll .\build /Y /E /S /H /I /q
xcopy ..\freeglut\bin\*.dll .\build /Y /E /S /H /I /q
gcc -Lbuild/ -Ilib/ -g main.c -o build/main.exe -lfreeglut -lopengl32 -lglu32 -lcomdlg32 -lfreetype