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

#define VERSION "0.1"

char** actionEntries = NULL;
size_t actionCap = 32;
size_t actionCount = 0;

char path[4096];
int page = 0;
int pageSize;
int selected = 0;
int secondSelect = -1;

char** curDirEntries = NULL;
size_t curDirCap = 32;
size_t curDirCount = 0;

bool up = true;

void cleanup_actions() {
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
					return;
				}
			}
			
			return;
		}
	
		
		actionEntries[actionCount] = strdup(buf);
		actionCount++;
	}
	else return;
}

void re_calc_sizes() {
	get_size();
	pageSize = HEIGHT-2;
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
	sprintf(buf1, "%d OUT OF %d PAGES", page+1, 1+(int)curDirCount/pageSize);
	
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
			case '[':
				secondSelect = selected;
				break;
			case ']':
				int aSel = secondSelect;
				int bSel = selected;
				
				int saveSel = selected;
				
				if (bSel < aSel) {
					int tempSel = aSel;
					aSel = bSel;
					bSel = tempSel;
				}
				
				for (int i = aSel; i <= bSel; i++) {
					selected = i;
					add2action();
				}
				
				secondSelect = -1;
				selected = saveSel;
				
				break;
			case 'm':
				if (selected == -1) break;
				if (actionEntries == NULL) break;
				
				char* targetBase = curDirEntries[selected];
				if (!is_dir(targetBase)) break;
				
				for (int i = 0; i < actionCount; i++) {
					char* dup = strdup(actionEntries[i]);
					char* base = basename(dup);
					
					char target[4096];
					sprintf(target, "%s/%s", targetBase, base);
					
					rename(actionEntries[i], target);
					
					free(dup);
				}
				
				cleanup_actions();
				update_fs();
				break;
			case 'c':
				if (actionEntries == NULL) break;
				if (selected == -1) break;
				
				if (!is_dir(curDirEntries[selected])) break;
				
				char* nodeB = escape(curDirEntries[selected]);
				
				for (int i = 0; i < actionCount; i++) {
					char cmd[4096];
					char* nodeA = escape(actionEntries[i]);
					
					
					sprintf(cmd, "cp -r \"%s\" \"%s\"", nodeA, nodeB);
					system(cmd);
					
					free(nodeA);
				}
				
				free(nodeB);
				
				cleanup_actions();
				update_fs();
				break;
			case 'r':
				if (actionEntries == NULL) break;
				
				char* sure = askprompt("Are you sure? (yes/no) ");
				if (sure == NULL) break;
				if (strcmp(sure, "yes") != 0) {
					free(sure);
					break;
				}
				free(sure);
				
				for (int i = 0; i < actionCount; i++) {
					char cmd[4096];
					
					char* nodeA = escape(actionEntries[i]);
					
					sprintf(cmd, "rm -rf \"%s\"", nodeA);
					
					system(cmd);
					free(nodeA);
				}
				
				cleanup_actions();
				update_fs();
				break;
			case 'n':
				if (actionEntries == NULL) break;
				
				char* newName = askline();
				
				if (newName == NULL) break;
				
				if (actionCount == 1) {
					char* tpath = strdup(actionEntries[0]);
					char ntpath[4096];
					
					sprintf(ntpath, "%s/%s", dirname(tpath), newName);
					rename(actionEntries[0], ntpath);
					
					free(tpath);
				}
				else {
					for (int i = 0; i < actionCount; i++) {
						char* tpath = strdup(actionEntries[0]);
						char ntpath[4096];
						
						sprintf(ntpath, "%s/%s%d", dirname(tpath), newName, i+1);
						free(tpath);
						rename(actionEntries[i], ntpath);
					}
				}
				
				cleanup_actions();
				
				update_fs();
				break;
			case 'u':
				secondSelect = -1;
				selected = -1;
				cleanup_actions();
				update_fs();
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
				if (!is_dir(curDirEntries[selected])) break;
				
				chdir(curDirEntries[selected]);
				getcwd(path, sizeof(path));
				update_fs();
				break;
			case 'x':
				char* fPattern = askline();
				if (fPattern == NULL) break;
				bool matchFound = false;
				
				for (int i = 0; i < curDirCount; i++) {
					if (matches(fPattern, curDirEntries[i])) {
						selected = i;
						page = selected/pageSize;
						matchFound = true;
						break;
					}
				}
				
				if (!matchFound) {
					printf("\x1b[1A\x1b[%dD\x1b[2K\x1b[41mNot found\x1b[0m", WIDTH); fflush(stdout);
					getc(stdin);
					
					free(fPattern);
				}
				break;
			case 'e':
				char* gotoDir = askline();
				if (gotoDir == NULL) break;
				
				if (!is_dir(gotoDir)) break;
				
				chdir(gotoDir);
				getcwd(path, sizeof(path));
				update_fs();
				
				free(gotoDir);
				break;
			case ';':
				execute(false);
				update_fs();
				break;
			case 'v':
				char* newSettings = askline();
				
				process_settings(newSettings);
				
				free(newSettings);
				update_fs();
				break;
			case 'p':
				execute(true);
				update_fs();
				break;
			case 'i':
				add2action();
				break;
			case 'l':
				if (selected == -1) {
					selected = 0;
				}
				
				int max = curDirCount/pageSize + 1;
				if (page+1<max) {
					if (selected + pageSize < curDirCount)
						selected += pageSize;
					else
						selected = curDirCount-1;
					page++;
				}
				break;
			case 'k':
				if (selected == -1) {
					selected = 0;
				}
				
				if (page-1>=0) {
					selected -= pageSize;
					page--;
				}
				break;
			case 'y':
				char* format = askline();
				if (format == NULL) break;
				int tempSel = selected;
				
				for (int i = 0; i < curDirCount; i++) {
					if (matches(format, curDirEntries[i])) {
						selected = i;
						add2action();
					}
				}
				
				selected = tempSel;
				
				free(format);
				
				break;
			case 'f':
				char* line = askline();
				if (line == NULL) break;
			    
				if (selected != -1)
					selected = 0;
			    
				size_t newEntriesCur = 0;
				char** newEntries = malloc(sizeof(char*)*curDirCap);
				
				for (int i = 0; i < curDirCount; i++) {
					if (matches(line, curDirEntries[i])) {
						newEntries[newEntriesCur] = curDirEntries[i];
						newEntriesCur++;
					}
					else {
						free(curDirEntries[i]);
						curDirEntries[i] = NULL;
					}
				}
				free(line);
				
				if (newEntriesCur == 0) {
					free(newEntries);
					printf("\x1b[1A\x1b[2K\x1b[%dD", WIDTH);
					printf("\x1b[41mNot found\x1b[0m");
					update_fs();
					getc(stdin);
					break;
				}
				
				free(curDirEntries);
				curDirEntries = newEntries;
				curDirCount = newEntriesCur;
				
				page = 0;
				if (selected >= curDirCount) {
					selected = 0;
				}
				if (selected >= pageSize) {
					selected = 0;
					page = 0;
				}
				
				break;
		}
		
		re_calc_sizes();
		printf("\x1b[2J\x1b[H");
	}
	
	toggle_input();
	toggle_output();
	return 0;
}
