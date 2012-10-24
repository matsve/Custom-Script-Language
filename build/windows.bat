@echo off

set GIT="C:/Program Files/Git/bin/git.exe"

:input
echo Please choose one of the following:
echo   Building
echo     1. Build Debug
echo     2. Build Release
echo.
echo     5. Clean
echo.
echo   Project
echo     10. Create for GNU Make
echo.
echo     0. Exit
echo.
set INPUT=1
set /P INPUT=Target (1): %=%
::if "%INPUT%"=="" goto cdebug
if "%INPUT%"=="0" goto theendreally
if "%INPUT%"=="1" goto build_debug
if "%INPUT%"=="2" goto build_release
if "%INPUT%"=="5" goto build_clean
if "%INPUT%"=="10" goto proj_gmake

echo Wrong input!
goto theend

:build_debug
echo Building Debug configuration
mingw32-make config=debug
echo Done with the building
goto theend

:build_release
echo Building Release configuration
mingw32-make config=release
echo Done with the building
goto theend

:build_clean
mingw32-make clean config=debug
mingw32-make clean config=release
goto theend

:proj_gmake
premake4 --file=premake4.lua gmake
goto theend

:theend
pause
cls
goto input

:theendreally
echo Ending shell script
