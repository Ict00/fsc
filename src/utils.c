#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>

int WIDTH, HEIGHT;

bool hideHidden = false;
bool showFiles = true;
bool showDirs = true;

void get_size() {
	struct winsize w;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
		WIDTH = w.ws_col;
		HEIGHT = w.ws_row-2;
		
	} else {
	}
}

bool is_file(const char *path) {
	struct stat path_stat;

	if (stat(path, &path_stat) != 0)
		return false;

	return S_ISREG(path_stat.st_mode);
}

bool is_symlink(const char *path) {
	struct stat path_stat;

	if (stat(path, &path_stat) != 0)
		return false;

	return S_ISLNK(path_stat.st_mode);
}

bool is_dir(const char *path) {
	struct stat path_stat;
	
	if (stat(path, &path_stat) != 0)
		return false;

	return S_ISDIR(path_stat.st_mode);
}

bool is_executable(const char *path) {
	return access(path, X_OK) == 0;
}

char** filter(char** entries, size_t count, size_t* newCount) {
	char** newEntries = malloc(sizeof(char*)*count);
	size_t c = 0;
	
	for (int i = 0; i < count; i++) {
		if ((strcmp(entries[i], ".") == 0 || strcmp(entries[i], "..") == 0)) {
			goto filterPassed;
		}
		
		if (showFiles) {
			if (is_file(entries[i])) {
				if(hideHidden) {
					if (entries[i][0] != '.') {
						goto filterPassed;
					}
					
					free(entries[i]);
					continue;
				}
				goto filterPassed;
			}
		}
		else {
			if (is_file(entries[i])) {
				free(entries[i]);
				continue;
			}
		}
		
		if (showDirs) {
			if (is_dir(entries[i])) {
				if(hideHidden) {
					if (entries[i][0] != '.') {
						goto filterPassed;
					}
					
					free(entries[i]);
					continue;
				}
				goto filterPassed;
			}
		}
		else {
			if (is_dir(entries[i])) {
				free(entries[i]);
				continue;
			}
		}
		
filterPassed:
		newEntries[c] = entries[i];
		c++;
	}
	
	*newCount = c;
	free(entries);
	
	return newEntries;
}

void sort(char **entries, size_t count) {
	for (int i = 0; i < count - 1; i++) {
		if (strcmp(entries[i], entries[i+1]) > 0) {
			// swap
			char* temp = entries[i];
			entries[i] = entries[i+1];
			entries[i+1] = temp;
		}
	}
	
	if (!is_sorted(entries, count)) sort(entries, count);
}

bool is_sorted(char **entries, size_t count) {
	for (int i = 0; i < count - 1; i++) {
		if (strcmp(entries[i], entries[i+1]) > 0) return false;
	}
	
	return true;
}

int longest(char **entries, size_t count) {
	int l = strlen(entries[0]);
	
	for (int i = 1; i < count; i++) {
		int nl = strlen(entries[i]);
		
		if (nl > l) l = nl;
	}
	
	return l;
}

void toggle_output() {
	static bool enabled = true;
	
	if (enabled) {
		printf("\x1b[?1049h"); fflush(stdout);
		enabled = !enabled;
	}
	else {
		printf("\x1b[?1049l"); fflush(stdout);
		enabled = !enabled;
	}
}

void toggle_input() {
	static struct termios old, new;
	static bool enabled = true;
	
	if (enabled) {
		tcgetattr(STDIN_FILENO, &old);
		new = old;
		new.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &new);
		enabled = !enabled;
	}
	else {
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		enabled = !enabled;
	}
}

char* get_color(const char* path) {
	if (is_dir(path)) {
		if (strcmp(path, "..") == 0 || strcmp(path, ".") == 0) return "\x1b[47m   \x1b[0m";
		
		return "\x1b[46m   \x1b[0m";
	}
	if (is_symlink(path)) return "\x1b[45m   \x1b[0m";
	
	if (is_file(path)) {
		if (is_executable(path)) return "\x1b[42m   \x1b[0m";
	}
	return "\x1b[43m   \x1b[0m";
}

char* escape(const char* src) {
	int l = strlen(src);
	char* res = malloc(sizeof(char)*(l*2+1));
	int b = 0;
	for (int i = 0; i < l; i++) {
		if (src[i] == '"') {
			res[b] = '\\';
			b++;
		}
		res[b] = src[i];
		b++;
	}
	res[b] = 0;
	res = realloc(res, sizeof(char)*(b+1));
	
	return res;
}

static bool contains(const char* a, const char* b) {
	
	for (; *a != '*' && *a != ',' && *a != 0; a++) {
		if (*a == '\\') a++;
		if (*b == 0) return false;
		if (*a != *b) return false;
		b++;
	}
	if (*a == 0 && *b != 0) return false;
	return true;
}

bool matches(const char* expr, const char* src) {
	int ei = 0;
	int si = 0;
	
	if (strcmp(src, "..") == 0 || strcmp(src, ".") == 0) return false;
	
	for(; src[si] != 0; si++) {
		if (expr[ei] == 0) return false;
		if (expr[ei] == '*') {
			if (contains(&expr[ei+1], &src[si])) { // src[si] == expr[ei+1]
				ei += 2;
			}
			continue;
		}
		
		if (expr[ei] == ',') {
			ei++;
			continue;
		}
		
		if (expr[ei] == '\\') {
			ei++;
		}
		
		if (src[si] == expr[ei]) {
			ei++;
		}
		else {
			return false;
		}
	}
	
	return (strlen(expr) == ei || (strlen(expr) == ei + 1 && expr[ei] == '*')) && strlen(src) == si;
}

void process_settings(const char* newSettings) {
	for (int i = 0; newSettings[i] != 0; i++) {
		switch (newSettings[i]) { 
			case 'H':
				hideHidden = true;
				break;
			case 'h':
				hideHidden = false;			
				break;
			case 'D':
				showDirs = false;
				break;
			case 'd':
				showDirs = true;		
				break;
			case 'F':
				showFiles = false;
				break;
			case 'f':
				showFiles = true;			
				break;
		}
	}
}
