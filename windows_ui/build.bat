@echo off
REM The portion that calls the vswhere.exe and so on... is basically copied from our lord and savior martin's github

setlocal enabledelayedexpansion

where /Q cl.exe || (
  set __VSCMD_ARG_NO_LOGO=1
  for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationPath') do set VS=%%i
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

if "%1" equ "debug" (
  set CL=/MTd /EHa /D_DEBUG=1 /Od /Zi /Fdui.pdb /fsanitize=address
  set LINK= /DEBUG /subsystem:console
) else (
  set CL=/GL /O1 /GS-
  set LINK=%LINK% /LTCG /OPT:REF /OPT:ICF libvcruntime.lib ucrt.lib /subsystem:windows
) 

IF NOT EXIST build mkdir build 
pushd build
 
rc.exe /nologo ../enet.rc
cl ../main.c ../enet.res /Feui.exe /FC /W3 /WX /MP %CL% /nologo /link  /INCREMENTAL:NO %LINK% /FIXED /merge:_RDATA=.rdata
popd
