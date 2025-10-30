#include "globals.h"
#include "common.h"
#include "utils.h"
#include <string.h>
#include <locale.h>
#include <stdio.h>
#include <dirent.h>

bool is_in_actions(const char* path) {
	if (actionEntries == NULL) return false;
	char abs[4096];
	
	if (realpath(path, abs) == NULL) return false;
	
	for (int i = 0; i < actionCount; i++) {
		if (actionEntries[i] == NULL) continue;
		
		if (strcmp(actionEntries[i], abs) == 0) {
			return true;
		}
	}
	
	return false;
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

char* get_prefix(const char* entry, int i) {
	if (secondSelect != -1) {
		int a = selected;
		int b = secondSelect;
		
		if (a > b) {
			int temp = a;
			a = b;
			b = temp;
		}
		
		if (i == a) {
			return "\x1b[47;30m[\x1b[0m";
		}
		
		if (i == b) {
			return "\x1b[47;30m]\x1b[0m";
		}
		
		if (i > a && i < b) {
			return "\x1b[47;30m|\x1b[0m";
		}
	}
	
	if (selected == i) {
		return "\x1b[47;30m>\x1b[0m";
	}
	
	if (is_in_actions(entry)) {
		return "\x1b[47;30m?\x1b[0m";
	}
	
	return " ";
}

void add2action() {
	if (selected == -1) return;
	
	if (actionEntries == NULL) {
		actionEntries = malloc(actionCap*sizeof(char*));
	}
	
	if (actionCount == actionCap) {
		actionCap *= 2;
		actionEntries = realloc(actionEntries, sizeof(char*)*actionCap);
	}
	char buf[4096];
	
	if (realpath(curDirEntries[selected], buf) != NULL) {
		if (is_in_actions(buf)) {
			for (int i = 0; i < actionCount; i++) {
				if (actionEntries[i] == NULL) continue;
				
				if (strcmp(actionEntries[i], buf) == 0) {
					free(actionEntries[i]);
					actionEntries[i] = NULL;
					visualActionCount--;
					return;
				}
			}
			
			return;
		}
	
		visualActionCount++;
		actionEntries[actionCount] = strdup(buf);
		actionCount++;
	}
	else return;
}

void recalc_termsize() {
	get_size();
	pageSize = HEIGHT-2;
}

void cleanup_actions() {
	visualActionCount = 0;
	
	if (actionEntries == NULL) {
		actionCap = 32;
		actionCount = 0;
		return;
	}
	
	for (int i = 0; i < actionCount; i++) {
		if (actionEntries[i] == NULL) continue;
		free(actionEntries[i]);
	}
	
	actionCap = 32;
	actionCount = 0;
	
	if (actionEntries != NULL) {
		free(actionEntries);
		actionEntries = NULL;
	}
}

void update_fs() {
	secondSelect = -1;
	for (int i = 0; i < curDirCount; i++) {
		if (curDirEntries[i] == NULL) continue;
		
		free(curDirEntries[i]);
	}
	
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
		curDirEntries[curDirCount] = strdup(entry->d_name);
		curDirCount++;
	}
	
	closedir(dir);
	curDirEntries = filter(curDirEntries, curDirCount, &curDirCount);
	
	
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
		printf("%s%-*s %s\n", get_prefix(curDirEntries[i], i), ln, curDirEntries[i], get_color(curDirEntries[i]));
	}
		
	char buf1[256];
	
	sprintf(buf1, "%d OUT OF %d PAGES | Selected: %zu", page+1, 1+(int)curDirCount/pageSize, visualActionCount);
	
	printf("\x1b[44m");
	bar(buf1, true);
	printf("\x1b[0m");
	
	printf("\x1b[104m");
	bar("FSC " VERSION, true);
	printf("\x1b[0m");

	fflush(stdout);
	
}

char* askprompt(char* prompt) {
	draw();
	toggle_input();
	printf("\x1b[%d;0H%s", HEIGHT+2, prompt); fflush(stdout);
	
	char* line = NULL;
	size_t lineLen = 0;
	if (getline(&line, &lineLen, stdin) == -1) {
		free(line);
		toggle_input();
		return NULL;
	}
	
	if (line[strlen(line)-1] == '\n')
		line[strlen(line)-1] = '\0';
	
	toggle_input();
	
	return line;
}

char* askline() {
	return askprompt("");
}

void execute(bool out) {
	char* line = askline();
	
	if (out) {
		printf("\x1b[2J\x1b[H"); fflush(stdout);
	}
	system(line);
	free(line);
		
	if (out)
		getc(stdin);
}
