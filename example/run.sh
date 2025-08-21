#!/bin/bash

# Get the script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Store original directory
ORIGINAL_DIR="$(pwd)"

# Change to script directory
cd "$SCRIPT_DIR"

# Check if --tests flag is provided
if [ "$1" = "--tests" ]; then
    echo "Running tests..."
    # Run each test file in the tests folder
    for f in ./tests/*.lang; do
        if [ -f "$f" ]; then
            echo "Running test: $f"
            ./example.exe "$f"
        fi
    done
else
    # Compile the C files
    gcc -o example.exe *.c ../src/*.c -lm -ldl -g
    # Run the example
    ./example.exe
fi

# Change back to original directory
cd "$ORIGINAL_DIR"
