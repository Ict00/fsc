#include "utils.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <unistd.h>
#include <libgen.h>

char path[4096];
int page = 0;
int pageSize;
int selected = 0;

char** curDirEntries = NULL;
size_t curDirCap = 32;
size_t curDirCount = 0;

bool up = true;

void re_calc_sizes() {
	get_size();
	pageSize = HEIGHT-2;
}

void update_pages() {
	
}

void update_fs() {
	free(curDirEntries);
	curDirCap = 32;
	curDirEntries = malloc(sizeof(char*)*curDirCap);
	curDirCount = 0;
	
	DIR* dir;
	struct dirent* entry;
	
	dir = opendir(path);
	
	if (dir == NULL) exit(-1);
	
	while((entry = readdir(dir)) != NULL) {
		if (curDirCount == curDirCap) {
			curDirCap *= 2;
			curDirEntries = realloc(curDirEntries, sizeof(char*)*curDirCap);
		}
		curDirEntries[curDirCount] = entry->d_name;
		curDirCount++;
	}
	
	closedir(dir);
	sort(curDirEntries, curDirCount);
	
	page = 0;
	
	if (selected >= curDirCount) {
		selected = 0;
	}
}

void bar(char* str) {
	int len = strlen(str);
	
	for (int x = 0; x < WIDTH; x++)
		if (len > x)
			printf("%c", str[x]);
		else
			printf(" ");
	printf("\n");
}

void draw() {
	printf("\x1b[41m");
	
	bar(path);
	
	printf("\x1b[0m");
	
	char format[256];
	sprintf(format, "%%s%%-%ds %%s\n", longest(curDirEntries, curDirCount)+3);
	
	for (int i = page*pageSize; i < ((page+1)*pageSize); i++) {
		if (i >= curDirCount) {
			printf("\n");
			continue;
		}
		printf(format, selected == i ? "\x1b[47;30m>\x1b[0m" : " ", curDirEntries[i], is_file(curDirEntries[i]) ? "\x1b[43m   \x1b[0m" : "\x1b[46m   \x1b[0m");
	}
		
	char buf1[256];
	sprintf(buf1, "%d OUT OF %d PAGES", page+1, 1+(int)curDirCount/pageSize);
	
	printf("\x1b[44m");
	bar(buf1);
	printf("\x1b[0m");

	fflush(stdout);
	
}

int main() {
	printf("\x1b[2J\x1b[H");
	toggle_input();
	get_size();
	pageSize = HEIGHT-2;
	getcwd(path, sizeof(path));
	update_fs();
	
	while(up) {
		draw();
		
		char input =  tolower(getc(stdin));
		printf("\x1b[2J\x1b[H"); fflush(stdout);
		
		switch (input) {
			case 'q':
				up = false;
				break;
			case 'u':
				selected = -1;
				break;
			case 'w':
				if (selected == -1)
					selected = 0;
				else {
					if (selected - 1 >= 0)
						selected--;
					if (selected < (page)*pageSize) {
						page--;
					}
				}
				break;
			case 's':
					if (selected == -1)
						selected = 0;
					else {
						if (selected + 1 < curDirCount)
							selected++;
						if (selected >= (page+1)*pageSize) {
							page++;
						}
					}
					break;
			case 'a':
				char npath[4096];
				sprintf(npath, "%s/..", path);
				chdir(npath);
				getcwd(path, sizeof(path));
				update_fs();
				break;
			case 'd':
				if (selected == -1) break;
				chdir(basename(curDirEntries[selected]));
				getcwd(path, sizeof(path));
				update_fs();
				break;
		}
		
		re_calc_sizes();
	}
	
	toggle_input();
	return 0;
}
