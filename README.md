# textfuck
An ncurses-based text editor inspired by Brainfuck written in C++

> "this is horrid" — some guy on discord

## Syntax
The syntax is heavily inspired by Brainfuck, with the addition of `q` to exit.

| Command | Meaning |
| ---: | :--- |
| `>` | Move cursor right one <br> Append character if at end of line |
| `<` | Move cursor left one <br> Prepend character if at start of line |
| `+` | Increment character by one <br> Remove character if it overflows |
| `-` | Decrement character by one <br> Remove character if it overflows |
| `.` | Write file to disk |
| `,` | Set one character from the keyboard |
| `[` | Move to the next line <br> Append line if at end of file |
| `]` | Move to the previous line <br> Prepend line if at start of file |
| `q` | Quit |

## Configuration
This is a WYSIWYG editor. What you see when you launch is the editor you get.

## Features
- Text editing
- Writing to disk
- An experience to remember

## Building
`make release` (default)

`make debug` - for debugging

### Dependencies
- Some C++ compiler which supports C++20 and `#pragma once`
- ncurses (tested only with `6.4`)
