#include "cmds.h"
#include "utils.h"
#include "globals.h"
#include "common.h"
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/dir.h>
#include <unistd.h>
#include <libgen.h>

int main() {
	printf("\x1b[2J\x1b[H");
	toggle_input();
	get_size();
	jumpMark[0] = 0;
	pageSize = HEIGHT-2;
	getcwd(path, sizeof(path));
	update_fs();
	
	while(up) {
		draw();
		
		char input =  tolower(getc(stdin));
		printf("\x1b[2J\x1b[H"); fflush(stdout);
		
		switch (input) {
			CMD_CASE('q', up = false);
			CMD_CASE('[', secondSelect = selected);
			CMD_CASE(']', _close_multiple_sel());
			CMD_CASE('m', _move());
			CMD_CASE('c', _copy());
			CMD_CASE('r', _remove());
			CMD_CASE('n', _rename());
			CMD_CASE('u', _deselect());
			CMD_CASE('w', _move_up());
			CMD_CASE('s', _move_down());
			CMD_CASE('a', _goto_parent());
			CMD_CASE(10, _goto_current()); // Enter
			CMD_CASE('d', _goto_current());
			CMD_CASE('e', _goto_path());
			CMD_CASE('x', _set_current_by_pattern());
			CMD_CASE(';', _exec_without_output());
			CMD_CASE('p', _exec_with_output());
			CMD_CASE('v', _change_settings());
			CMD_CASE('i', add2action());
			CMD_CASE('l', _move_page_down());
			CMD_CASE('k', _move_page_up());
			CMD_CASE('y', _select_by_pattern());
			CMD_CASE('f', _search_by_pattern());
			CMD_CASE('b', _set_jump_mark());
			CMD_CASE('j', _goto_jump_mark());
		}
		
		recalc_termsize();
		printf("\x1b[2J\x1b[H");
	}
	
	toggle_input();
	toggle_output();
	return 0;
}
