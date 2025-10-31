#pragma once

#define CMD_CASE(symbol, expr) case symbol: { expr; } break;

void _close_multiple_sel();
void _move();
void _copy();
void _remove();
void _rename();
void _deselect();
void _move_up();
void _move_down();
void _goto_parent();
void _goto_current();
void _goto_path();
void _set_current_by_pattern();
void _exec_with_output();
void _exec_without_output();
void _change_settings();
void _move_page_up();
void _move_page_down();
void _select_by_pattern();
void _search_by_pattern();
void _set_jump_mark();
void _goto_jump_mark();
