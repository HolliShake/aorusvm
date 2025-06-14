#!/bin/bash

# Common flags and files
if [[ "$*" == *"--production"* ]]; then
    SHARED_FLAGS="-shared -fPIC -L. -lm -O3 -s"
    DEBUG_FLAG=""
else
    SHARED_FLAGS="-shared -fPIC -L. -lm -g"
    DEBUG_FLAG="-g"
fi

STATIC_FLAGS="-static"
SRC_FILES="aorusvm.c ./src/*.c"
OUTPUT_DIR="./bin"
OUTPUT_SHARED_LIB="$OUTPUT_DIR/libaorusvm.so"
OUTPUT_STATIC_LIB="$OUTPUT_DIR/libaorusvm.a"
OUTPUT_EXE="aorusvm.exe"

# Create bin directory if it doesn't exist
mkdir -p $OUTPUT_DIR

if [[ "$*" == *"--warn"* ]]; then
    WARNING_FLAG="-Wall"
else
    WARNING_FLAG="-w"
fi

# Build shared library
gcc $WARNING_FLAG $SHARED_FLAGS $SRC_FILES -o $OUTPUT_SHARED_LIB

# Build static library
gcc $WARNING_FLAG $DEBUG_FLAG -c $SRC_FILES
ar rcs $OUTPUT_STATIC_LIB *.o
rm *.o

# Build executable
gcc $WARNING_FLAG $DEBUG_FLAG aorusvm.c -L$OUTPUT_DIR -Wl,-rpath=$OUTPUT_DIR -laorusvm -lm -ldl -o $OUTPUT_EXE