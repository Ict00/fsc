#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>

int WIDTH, HEIGHT;

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

bool is_dir(const char *path) {
	struct stat path_stat;
	
	if (stat(path, &path_stat) != 0)
		return false;

	return S_ISDIR(path_stat.st_mode);
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
