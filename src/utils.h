#pragma once
#include <stdbool.h>
#include <stddef.h>

extern int WIDTH, HEIGHT;

void get_size();
bool is_dir(const char* path);
bool is_file(const char* path);
void sort(char** entries, size_t count);
bool is_sorted(char** entries, size_t count);
int longest(char** entries, size_t count);
void toggle_input();
