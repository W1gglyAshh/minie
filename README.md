# MINIE

## A minimal text editor

This project is licensed under GPL v2

It is a cross-platform, light weight text editor written in C++, inspired by kilo and helix-editor.

## Usage

- `minie <filename>`

- There's no NORMAL, INSERT or VISUAL modes like vim. After opening up, you can directly edit the file.

- It does have some vim features. Press ESC to open the command palette. You can type in some simple vim command (as of now, there's just `w <filename>` `w` `wq` `q` `q!`). More command will be supported in the future.

- I'm planning to add more feature to it to make minie a more powerful code editor.
There will be lsp integration, syntax highlighting and smart indent in the future.

## Build from source

1. Make sure you have CMake installed on your system.

2. Navigate to the project's root directory, create a `build` folder and open your system terminal in it.

3. Type command `cmake ..` and wait for it to generate build files. After that, you can use `cmake --build .` to build the executable.
