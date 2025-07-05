#!/bin/bash

# Get the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Change to script directory
cd "$SCRIPT_DIR"

# Compile the C files
gcc -o example.exe *.c ../src/*.c -lm -ldl -g

# Run the example
./example.exe

# Change back to original directory
cd - > /dev/null
