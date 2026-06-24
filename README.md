# 2D Graphics Editor (C)

A terminal-based 2D graphics editor written in C that draws shapes on a character canvas using `*` and `_`.

## Features

- **Draw shapes:** Circle, Rectangle, Line, Triangle
- **Manage objects:** Add, Delete, Modify
- **Canvas:** 25 × 70 character grid rendered with borders
- **Save:** Export the canvas to a `.txt` file
- Shapes are stored in an object list and re-rendered on every change

## Building

```bash
make
```

Or manually:

```bash
gcc -Wall -Wextra -std=c99 -o graphics_editor graphics_editor.c -lm
```

## Running

```bash
./graphics_editor
```

## Usage

The program shows a numbered menu:

```
1. Add Circle
2. Add Rectangle
3. Add Line
4. Add Triangle
5. Delete Object
6. Modify Object
7. List Objects
8. Display Canvas
9. Save Canvas to File
0. Exit
```

- Coordinates are **row, col** (row 0 is the top).
- Fill character is either `*` or `_`.
- Each object gets a numeric **ID** shown in the object list, which you use to delete or modify it.

## Example session

```
Choice: 1
Fill character ('*' or '_'): *
Center row (0-24): 12
Center col (0-69): 35
Radius: 8
Circle added with ID 0.
```

## Files

| File                | Purpose                    |
|---------------------|----------------------------|
| `graphics_editor.c` | Main source                |
| `Makefile`          | Build rules                |
| `README.md`         | This file                  |

## Algorithm notes

- **Line** – Bresenham's line algorithm (integer-only, no floating point)
- **Circle** – Midpoint circle algorithm; horizontal radius doubled to compensate for terminal character aspect ratio
- **Rectangle** – Four straight edges via the line routine
- **Triangle** – Three edges connecting the given vertices
