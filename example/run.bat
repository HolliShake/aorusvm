@echo off

REM Get the script directory
set SCRIPT_DIR=%~dp0

REM Change to script directory
cd /d "%SCRIPT_DIR%"

REM Compile the C files
gcc -o example.exe *.c ../src/*.c -lm -ldl -g

REM Run the example
example.exe

REM Change back to original directory
cd /d "%OLDPWD%" 2>nul || cd /d %USERPROFILE%
