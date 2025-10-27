#include "utils.h"
#include <ctype.h>
#include <dirent.h>
#include <locale.h>
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
		//if (strcmp(entry->d_name, "..") == 0 || strcmp(entry->d_name, ".") == 0) continue;
		
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
	if (selected >= pageSize) {
		selected = 0;
		page = 0;
	}
}

void bar(char* str, bool newLine) {
	int len = strlen(str);
	
	for (int x = 0; x < WIDTH; x++)
		if (len > x)
			printf("%c", str[x]);
		else
			printf(" ");
	if (newLine)
		printf("\n");
}

void draw() {
	setlocale(LC_ALL, 0);
	printf("\x1b[41m");
	
	bar(path, true);
	
	printf("\x1b[0m"); fflush(stdout);
	
	int ln = longest(curDirEntries, curDirCount)+3;
	
	if (ln >= WIDTH)
		ln = WIDTH - 6;
		
	for (int i = page*pageSize; i < ((page+1)*pageSize); i++) {
		if (i >= curDirCount) {
			printf("\n");
			continue;
		}
		printf("%s%-*s %s\n", selected == i ? "\x1b[47;30m>\x1b[0m" : " ", ln, curDirEntries[i], get_color(curDirEntries[i]));
	}
		
	char buf1[256];
	sprintf(buf1, "%d OUT OF %d PAGES", page+1, 1+(int)curDirCount/pageSize);
	
	printf("\x1b[44m");
	bar(buf1, true);
	printf("\x1b[0m");
	
	printf("\x1b[104m");
	bar("F - Search, WASD ENTER - Navigate, P - Run CMD, ` - Run CMD (No output)", true);
	printf("\x1b[0m");

	fflush(stdout);
	
}

void execute(bool out) {
	draw();
	toggle_input();
	
	printf("\x1b[%d;0H", HEIGHT+2); fflush(stdout);
	
	char* line = NULL;
	size_t lineLen = 0;
	
	if (getline(&line, &lineLen, stdin) == -1) {
		free(line);
		toggle_input();
		return;
	}
	
	if (out) {
		printf("\x1b[2J\x1b[H"); fflush(stdout);
	}
	system(line);
	free(line);
	
	toggle_input();
	
	if (out)
		getc(stdin);
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
			case 10:
			case 'd':
				if (selected == -1) break;
				chdir(basename(curDirEntries[selected]));
				getcwd(path, sizeof(path));
				update_fs();
				break;
			case 'r':
				update_fs();
				break;
			case '`':
				execute(false);
				update_fs();
				break;
			case 'p':
				execute(true);
				update_fs();
				break;
			case 'f':
				draw();
				toggle_input();
				printf("\x1b[%d;0H", HEIGHT+2); fflush(stdout);
				
				char* line = NULL;
				size_t lineLen = 0;
				if (getline(&line, &lineLen, stdin) == -1) {
					free(line);
					toggle_input();
					break;
				}
				
				if (line[strlen(line)-1] == '\n')
					line[strlen(line)-1] = '\0';
			    
				if (selected != -1)
					selected = 0;
			    
				size_t newEntriesCur = 0;
				char** newEntries = malloc(sizeof(char*)*curDirCap);
				
				for (int i = 0; i < curDirCount; i++) {
					if (strstr(curDirEntries[i], line) != NULL) {
						newEntries[newEntriesCur] = curDirEntries[i];
						newEntriesCur++;
					}
				}
				free(line);
				
				if (newEntriesCur == 0) {
					free(newEntries);
					printf("\x1b[1A\x1b[2K\x1b[%dD", WIDTH);
					printf("\x1b[41m");
					bar("Not found", false);
					printf("\x1b[0m");
					toggle_input();
					getc(stdin);
					break;
				}
				
				free(curDirEntries);
				curDirEntries = newEntries;
				curDirCount = newEntriesCur;
				
				toggle_input();
				break;
		}
		
		re_calc_sizes();
		printf("\x1b[2J\x1b[H");
	}
	
	toggle_input();
	return 0;
}
