# FSC
FS Control is simple TUI file manager

# Usage
## Controls

* I - Add/Remove current file/dir to action list
* R - Remove all the files/dirs in action list
* C - Copy all the files/dirs in action list to selected dir
* M - Move all the files/dirs in action list to selected dir
* N - Rename all the files/dirs in action list
* F - Search for file in current dir
* P - Run CMD (With output)
* ` - Run CMD (No output)
* U - Unselect all
* WS - Move cursor up/down
* A - Move to parent directory of current dir
* D/Enter - Change current dir to selected dir

## Colors
Each color stands for element's type:
* Yellow - File
* Green - Executable
* Cyan - Directory
* White - . or .. only
* Magenta - Symlink

# Quick Start

## Supported platforms
* [x] Linux
* [x] Mac OS (Need tests)
* [ ] Windows

## Step 1: Clone repository
```bash
$ git clone https://github.com/Ict00/fsc
```

## Step 2: Build it using MPT
```bash
$ mpt build
```

## Step 3 (Optional): Put `fsc` binary into directory that's listed in `PATH` variable (for better experience) 

## Step 4: Use it
```bash
$ ./fsc
```
