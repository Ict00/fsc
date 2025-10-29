# FSC
FS Control is a TUI file manager
(Might still be buggy)

# Usage
## Controls

### Navigation
* **W/S** - Move cursor up/down
* **A** - Move to parent directory
* **D/Enter** - Enter selected directory
* **K/L** - Page up/down navigation
* **U** - Unselect all and clear action list

### File Operations
* **I** - Add/Remove current file/dir to action list (marked with `?`)
* **R** - Remove all files/dirs in action list (with confirmation)
* **C** - Copy all files/dirs in action list to selected directory
* **M** - Move all files/dirs in action list to selected directory
* **N** - Rename all files/dirs in action list (single file: rename, multiple: numbered)

### Search & Filter
* **F** - Search for files in current directory (supports wildcards `*`)
* **Y** - Mass select files matching pattern
* **V** - View settings - toggle display filters:
  - `h`/`H` - show/hide hidden files
  - `d`/`D` - show/hide directories  
  - `f`/`F` - show/hide regular files

### System Commands
* **P** - Run command (with output)
* **;** - Run command (no output)
* **Q** - Quit

## Display Features
* **Color coding**:
  - Yellow - Regular file
  - Green - Executable file
  - Cyan - Directory
  - White - `.` or `..` entries
* **Selection indicators**:
  - `>` - Currently selected item
  - `?` - Item in action list
* **Pagination** - Handles large directories efficiently

## Search Patterns
Supports advanced pattern matching:
- `*` - Wildcard character
- `,` - Multiple conditions
- `\` - Escape special characters

# Quick Start

## Requirements
Clang with C23 support

## Supported Platforms
* [x] Linux
* [x] Mac OS (Testing needed)
* [ ] Windows

## Installation Options

### Option 1: Using MPT (My Build System)
```bash
$ git clone https://github.com/Ict00/fsc
$ cd fsc
$ mpt build
```

### Option 2: Using Make
```bash
$ git clone https://github.com/Ict00/fsc  
$ cd fsc
$ make build
```

## Installation (Optional)
For best experience, add `fsc` to your PATH:
```bash
$ sudo cp fsc /usr/local/bin/  # System-wide
# OR
$ cp fsc ~/.local/bin/         # User-specific
```

## Running
```bash
$ ./fsc
```
