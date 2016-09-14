@echo off
@setlocal enabledelayedexpansion
goto :%1

:x64
@set LIB=
@set INCLUDE=
@call "!MSDEV_HOME!\vcvarsall.bat" amd64
@set INCLUDE=!INCLUDE_X64!;!INCLUDE!;
@set LIB=!LIB_X64!;!LIB!;
@nmake x64 
@exit !ERRORLEVEL!

:cppcheck
@set INCLUDE=
@call "!MSDEV_HOME!\vcvarsall.bat" x86
@set INCLUDE=!INCLUDE_X86!;!INCLUDE!
@set "INCLUDE=%INCLUDE:;=,%"
@set "INCLUDE=%INCLUDE: =|%"
@set CPPCHECK_I_VALUE=
@for %%P in (!INCLUDE!) do (
   @set DIR=%%P
   @set "DIR=!DIR:|= !"
   @if exist !DIR! (@set CPPCHECK_I_VALUE=!CPPCHECK_I_VALUE! -i"!DIR!" )
)
::@set CPPCHECK_I_VALUE=
@nmake /nologo cppcheck
@exit !ERRORLEVEL!

@endlocal