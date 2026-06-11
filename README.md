# 2D Graphics Editor

A simple menu-driven 2D graphics editor written in C.

## Overview

This project uses a 2D character array as the drawing canvas. The canvas is initially filled with `_` characters, and graphical objects are drawn using `*`.

Supported features:
- Draw lines, rectangles, circles, and triangles interactively with the    canvas using - 'WASD' or arrow keys.
- Add objects to the picture
- Delete objects from the picture
- Modify object properties
- Save and load canvas state
- Debug logging for redraw diagnostics

## Files

- `graphics_editor.c` - Main source code for the graphics editor.
- `prompt.txt` - Prompt history and assistant response logs.
- `README.md` - Project overview and usage instructions.

## Build and Run

Use a C compiler to build the editor. For example with `gcc`:

```sh
gcc graphics_editor.c -o graphics_editor
./graphics_editor
```

## Usage

Run the program and follow the console menu prompts to create, edit, save, and load graphical objects.

## Notes

- The canvas size and object behavior are managed in `graphics_editor.c`.
- This editor uses simple character raster graphics for output.
