#!/bin/bash

# clear the terminal
clear

#create bin directory if it doesn't exist
if [ ! -d "bin" ]; then
    mkdir bin   
fi

# Remove the existing main executable if it exists
if [ -f "bin/main" ]; then
    rm bin/main
    echo "Removed existing executable."
fi

# Compile main.c with clang
gcc src/main.c -o bin/main -g -fsanitize=address -fsanitize=undefined -Iinclude -Llib -lraylib -lm -ldl -lpthread -lGL -Wall -lrt -lX11 -Wextra -Werror -std=c99 -pedantic-errors

# Run the compiled executable
if [ -f "bin/main" ]; then
    ./bin/main
else
    echo "Compilation failed. Executable not found."
fi