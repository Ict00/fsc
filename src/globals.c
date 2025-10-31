#include "globals.h"

#include <stdbool.h>

char** actionEntries = NULL;
size_t actionCap = 32;
size_t actionCount = 0;
size_t visualActionCount = 0;

char jumpMark[4096];
int jumpSel = -1;

char path[4096];
int page = 0;
int pageSize;
int selected = 0;
int secondSelect = -1;

char** curDirEntries = NULL;
size_t curDirCap = 32;
size_t curDirCount = 0;

bool up = true;
