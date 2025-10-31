#include "cmds.h"
#include "globals.h"
#include "common.h"
#include "utils.h"
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


void _close_multiple_sel() {
	if (secondSelect == -1) return;

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
}

void _move() {
	if (selected == -1) return;;
	if (actionEntries == NULL) return;
	
	char* targetBase = curDirEntries[selected];
	if (!is_dir(targetBase)) return;
	
	for (int i = 0; i < actionCount; i++) {
		if (actionEntries[i] == NULL) continue;
		
		char* dup = strdup(actionEntries[i]);
		char* base = basename(dup);
		
		char target[4096];
		sprintf(target, "%s/%s", targetBase, base);
		
		rename(actionEntries[i], target); 
		
		free(dup);
	}
	
	cleanup_actions();
	update_fs();
}

void _copy() {
	if (actionEntries == NULL) return;
	if (selected == -1) return;
	
	if (!is_dir(curDirEntries[selected])) return;
	
	char* nodeB = escape(curDirEntries[selected]);
	
	for (int i = 0; i < actionCount; i++) {
		if (actionEntries[i] == NULL) continue;
		
		char cmd[4096];
		char* nodeA = escape(actionEntries[i]);
		
		
		sprintf(cmd, "cp -r \"%s\" \"%s\"", nodeA, nodeB);
		system(cmd);
		
		free(nodeA);
	}
	
	free(nodeB);
	
	cleanup_actions();
	update_fs();
}

void _remove() {
	if (actionEntries == NULL) return;;
	
	char* sure = askprompt("Are you sure? (yes/no) ");
	if (sure == NULL) return;
	if (strcmp(sure, "yes") != 0) {
		free(sure);
		return;
	}
	free(sure);
	
	for (int i = 0; i < actionCount; i++) {
		if (actionEntries[i] == NULL) continue;
		char cmd[4096];
		
		char* nodeA = escape(actionEntries[i]);
		
		sprintf(cmd, "rm -rf \"%s\"", nodeA);
		
		system(cmd);
		free(nodeA);
	}
	
	cleanup_actions();
	update_fs();
}

void _rename() {
	if (actionEntries == NULL) return;
	
	char* newName = askline();
	
	if (newName == NULL) return;
	
	if (visualActionCount == 1) {
		if (actionEntries[0] == NULL) return;
		
		char* tpath = strdup(actionEntries[0]);
		char ntpath[4096];
		
		sprintf(ntpath, "%s/%s", dirname(tpath), newName);
		rename(actionEntries[0], ntpath);
		
		free(tpath);
	}
	else {
		for (int i = 0; i < actionCount; i++) {
			if (actionEntries[i] == NULL) continue;
			
			char* tpath = strdup(actionEntries[0]);
			char ntpath[4096];
			
			sprintf(ntpath, "%s/%s%d", dirname(tpath), newName, i+1);
			free(tpath);
			rename(actionEntries[i], ntpath);
		}
	}
	
	cleanup_actions();
	
	update_fs();
}

void _deselect() {
	secondSelect = -1;
	selected = -1;
	cleanup_actions();
	update_fs();
}

void _set_jump_mark() {
	jumpMark[0] = 0;
	sprintf(jumpMark, "%s", path);
	jumpSel = selected;
}

void _goto_jump_mark() {
	selected = jumpSel;
	chdir(jumpMark);
	getcwd(path, sizeof(path));

	update_fs();
	
	page = selected/pageSize;
}

void _move_up() {
	if (selected == -1)
		selected = 0;
	else {
		if (selected - 1 >= 0)
			selected--;
		if (selected < (page)*pageSize) {
			page--;
		}
	}
}

void _move_down() {
	if (selected == -1)
		selected = 0;
	else {
		if (selected + 1 < curDirCount)
			selected++;
		if (selected >= (page+1)*pageSize) {
			page++;
		}
	}
}

void _goto_parent() {
	char npath[4096];
	sprintf(npath, "%s/..", path);
	chdir(npath);
	getcwd(path, sizeof(path));
	update_fs();
}

void _goto_current() {
	if (selected == -1) return;
	if (!is_dir(curDirEntries[selected])) return;
	
	chdir(curDirEntries[selected]);
	getcwd(path, sizeof(path));
	update_fs();
}

void _goto_path() {
	char* gotoDir = askline();
	if (gotoDir == NULL) return;
	
	if (!is_dir(gotoDir)) return;
	
	chdir(gotoDir);
	getcwd(path, sizeof(path));
	update_fs();
	
	free(gotoDir);
}

void _set_current_by_pattern() {
	char* fPattern = askline();
	if (fPattern == NULL) return;
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
}

void _exec_without_output() {
	execute(false);
	update_fs();
}

void _exec_with_output() {
	execute(true);
	update_fs();
}

void _change_settings() {
	char* newSettings = askline();
	
	process_settings(newSettings);
	
	free(newSettings);
	update_fs();
}

void _move_page_down() {
	if (selected == -1) {
		selected = 0;
	}
	
	int max = curDirCount/pageSize;
	if (page+1<=max) {
		if (selected + pageSize < curDirCount)
			selected += pageSize;
		else
			selected = curDirCount-1;
		page++;
	}
}

void _move_page_up() {
	if (selected == -1) {
		selected = 0;
	}
	
	if (page-1>=0) {
		selected -= pageSize;
		page--;
	}
}

void _select_by_pattern() {
	char* format = askline();
	if (format == NULL) return;
	int tempSel = selected;
	
	for (int i = 0; i < curDirCount; i++) {
		if (matches(format, curDirEntries[i])) {
			selected = i;
			add2action();
		}
	}
	
	selected = tempSel;
	
	free(format);
}

void _search_by_pattern() {
	char* line = askline();
	if (line == NULL) return;
    
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
		return;
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
}
