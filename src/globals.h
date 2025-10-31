#pragma once

#include <stdbool.h>
#include <stdlib.h>

extern char** actionEntries;
extern size_t actionCap;
extern size_t actionCount;
extern size_t visualActionCount;

extern char jumpMark[4096];
extern int jumpSel;

extern char path[4096];
extern int page;
extern int pageSize;
extern int selected;
extern int secondSelect;

extern char** curDirEntries;
extern size_t curDirCap;
extern size_t curDirCount;

extern bool up;
