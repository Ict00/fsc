#pragma once

#include <stdbool.h>
#define VERSION "0.1"


void add2action();
bool is_in_actions(const char* path);
void bar(char* str, bool newLine);
char* get_prefix(const char* entry, int i);
void recalc_termsize();
void cleanup_actions();
void update_fs();
void draw();
char* askprompt(char* prompt);
char* askline();
void execute(bool out);
