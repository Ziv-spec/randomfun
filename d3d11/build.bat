@echo off
REM The portion that calls the vswhere.exe and so on... I copied from our lord and savior martin's github

setlocal enabledelayedexpansion

where /Q cl.exe || (
  set __VSCMD_ARG_NO_LOGO=1
  for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft	.VisualStudio.Workload.NativeDesktop -property installationPath') do set VS=%%i
  if "!VS!" equ "" (
    echo ERROR: Visual Studio installation not found
    exit /b 1
  )  
  call "!VS!\VC\Auxiliary\Build\vcvarsall.bat" amd64 || exit /b 1
)

if "%VSCMD_ARG_TGT_ARCH%" neq "x64" (
  echo ERROR: please run this from MSVC x64 native tools command prompt, 32-bit target is not supported!
  exit /b 1
)

set original_path=%cd%

where /Q gameinput.h
if %errorlevel% neq 0 (
	REM call "C:\Program Files (x86)\Microsoft GDK\Command Prompts\GamingDesktopVars.cmd" GamingDesktopVS2022
)


if "%1" equ "debug" (
  set CL_FLAGS=/MTd /EHa /D_DEBUG=1 /Od /Zi 
  REM /fsanitize=address
  set LINK_FLAGS= /DEBUG /subsystem:console 
  set FXC=/O0
) else (
  set CL_FLAGS=/GL /O1 /GS-
  set LINK_FLAGS=%LINK_FLAGS% /LTCG /OPT:REF /OPT:ICF libvcruntime.lib /subsystem:windows
  set FXC=/O3
) 

cd %original_path%

IF NOT EXIST build mkdir build 
pushd build

rc.exe /nologo /fo enet.res ../resources/enet.rc 


REM cl ../main.c ../enet.res /Feui.exe /FC /W3 /WX /MP %CL_FLAGS% /nologo /link  /INCREMENTAL:NO %LINK_FLAGS% /FIXED /merge:_RDATA=.rdata

cl ../d3d11_example.cpp enet.res  /nologo /FC /W3 /WX %CL_FLAGS% /link /INCREMENTAL:NO %LINK_FLAGS% /FIXED /merge:_RDATA=.rdata

popd