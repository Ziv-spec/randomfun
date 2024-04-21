@echo off

mkdir build
pushd build

REM set PATH=C:\Program Files\CMake\bin;C:\Users\bkandler\Desktop\raylib\w64devkit\bin;%PATH%
set PATH=C:\Users\ziv\Desktop\zivbackup\raylib\raylib\w64devkit\bin;%PATH%
set include=-I..\include\ -I..\WpdPack\Include
set lib=-L..\lib -L..\WpdPack\Lib

REM Debug build with gcc compiler
REM gcc.exe ..\main.c -o ipassign.exe -std=c99 -O0 -DPLATFORM_DESKTOP -DDEBUG=1 %include% %lib% -lwpcap -liphlpapi -lraylib -lwinmm -lgdi32

REM Debug build with cl compiler
REM set wpack_lib=..\WpdPack\Lib\x64
REM cl /FC /nologo /Zi %include% ..\main.c /link /OUT:ipassign.exe /libpath:%wpack_lib%


REM RELEASE
C:\Users\ziv\Desktop\zivbackup\raylib\raylib\w64devkit\bin\gcc.exe ..\main.c -o ipassign.exe -std=c99 -O2 -DPLATFORM_DESKTOP -DDEBUG=0 %include% %lib% -lwpcap -liphlpapi -lraylib -lwinmm -lgdi32 -mwindows

popd
