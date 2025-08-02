@echo off

REM Common flags and files
if "%*"==*"--production"* (
    set SHARED_FLAGS=-shared -O2 -fPIC
    set DEBUG_FLAG=
) else (
    set SHARED_FLAGS=-shared -g -fPIC
    set DEBUG_FLAG=-g
)

set STATIC_FLAGS=-static
set SRC_FILES=aorusvm.c src/*.c
set OUTPUT_DIR=./bin
set OUTPUT_SHARED_LIB=%OUTPUT_DIR%\aorusvm.dll
set OUTPUT_STATIC_LIB=%OUTPUT_DIR%\libaorusvm.a
set OUTPUT_EXE=%OUTPUT_DIR%\aorusvm.exe

REM Create bin directory if it doesn't exist
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

if "%*"==*"--warn"* (
    set WARNING_FLAG=-Wall
) else (
    set WARNING_FLAG=-w
)

REM Build shared library
gcc %WARNING_FLAG% %SHARED_FLAGS% -Wl,--out-implib,%OUTPUT_DIR%/libaorusvm.dll.a %SRC_FILES% -o %OUTPUT_SHARED_LIB%

REM Build static library
gcc %WARNING_FLAG% %DEBUG_FLAG% -c %SRC_FILES%
ar rcs %OUTPUT_STATIC_LIB% *.o
rm -f *.o

REM Build executable
gcc %WARNING_FLAG% %DEBUG_FLAG% aorusvm.c -o %OUTPUT_EXE% -L%OUTPUT_DIR% -laorusvm
