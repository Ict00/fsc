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
		if(hideHidden) {
			if (entries[i][0] != '.' || (strcmp(entries[i], ".") == 0 || strcmp(entries[i], "..") == 0)) {
				goto filterPassed;
			}
			
			free(entries[i]);
			continue;
		}
filterPassed:
		newEntries[c] = entries[i];
		c++;
	}
	
	*newCount = c;
	
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

bool matches(const char* expr, const char* src) {
	int ei = 0;
	int si = 0;
	
	for(; src[si] != 0; si++) {
		if (expr[ei] == 0) return false;
		if (expr[ei] == ',') {
			ei++;
			continue;
		}
		if (expr[ei] == '*') {
			if (expr[ei+1] == src[si+1] && expr[ei+1] != 0) {
				ei++;
				si++;
			}
			else if(expr[ei+1] == src[si] && expr[ei+1] != 0) {
				ei++;
			}
			else continue;
		}
		
		if (expr[ei] == '\\') {
			ei++;
		}
		
		if (expr[ei] == src[si]) {
			ei++;
		}
		else {
			return false;
		}
	}
	
	return strlen(expr)-1 <= ei && strlen(src) == si;
}
