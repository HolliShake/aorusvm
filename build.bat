@echo off

REM Common flags and files
if "%*"==*"--production"* (
    set SHARED_FLAGS=/DLL /LIBPATH:. /O2
    set DEBUG_FLAG=
) else (
    set SHARED_FLAGS=/DLL /LIBPATH:. /DEBUG
    set DEBUG_FLAG=/DEBUG
)

set STATIC_FLAGS=/MT
set SRC_FILES=aorusvm.c src\*.c
set OUTPUT_DIR=.\bin
set OUTPUT_SHARED_LIB=%OUTPUT_DIR%\aorusvm.dll
set OUTPUT_STATIC_LIB=%OUTPUT_DIR%\aorusvm.lib
set OUTPUT_EXE=aorusvm.exe

REM Create bin directory if it doesn't exist
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

if "%*"==*"--warn"* (
    set WARNING_FLAG=/W3
) else (
    set WARNING_FLAG=/w
)

REM Build shared library
cl %WARNING_FLAG% %SHARED_FLAGS% %SRC_FILES% /Fe%OUTPUT_SHARED_LIB%

REM Build static library
cl %WARNING_FLAG% %DEBUG_FLAG% /c %SRC_FILES%
lib *.obj /OUT:%OUTPUT_STATIC_LIB%
del *.obj

REM Build executable
cl %WARNING_FLAG% %DEBUG_FLAG% aorusvm.c /Fe%OUTPUT_EXE% /link /LIBPATH:%OUTPUT_DIR% aorusvm.lib
