#include <PDCurses/curses.h>

#ifdef __U_BOOT__
#include <command.h>
#include <linux/compat.h>
#include <vsprintf.h>
#else
#include <stdlib.h>
#endif

#define str(s) #s
#define xstr(s) str(s)
#define MAX_LEN 30

typedef struct menuentry {
	int number;
	char *label;

	struct menuentry *previous;
	struct menuentry *next;
} menuentry;

typedef struct menustate {
	WINDOW *window;
	WINDOW *menu;

	menuentry *first_entry;
	menuentry *last_entry;
} menustate;

void redrawitem(menustate *state, int i, bool selected) {
	char item[MAX_LEN+3];
	menuentry *iter = state->first_entry;

	if (selected) {
		wattron(state->menu, A_STANDOUT);
	}

	while (iter->number != i) {
		if (!iter->next) { return; }
		iter = iter->next;
	}

	sprintf(item, " %-" xstr(MAX_LEN) "s ", iter->label);
	mvwprintw(state->menu, iter->number-1, 0, "%s", item);

	if (selected) {
		wattroff(state->menu, A_STANDOUT);
	}

	//touchwin(state->window);
}

void printmenu(menustate *state) {
	char item[MAX_LEN+3];
	menuentry *iter = state->first_entry;

	while (iter) {
		sprintf(item, " %-" xstr(MAX_LEN) "s ", iter->label);
		mvwprintw(state->menu, iter->number-1, 0, "%s", item);
		iter = iter->next;
	}

	redrawitem(state, 1, TRUE);
}

menuentry *init_entry(char* label, menuentry *previous) {
	menuentry *entry = malloc(sizeof(menuentry));
	entry->label = label;

	if (previous) {
		entry->number = previous->number + 1;
		previous->next = entry;
	}
	else {
		entry->number = 1;
	}

	entry->previous = previous;
	entry->next = NULL;

	return entry;
}

int menu_num_entries(menustate *state) {
	if (state->last_entry) {
		return state->last_entry->number;
	}
	else {
		return 0;
	}
}

void add_entry(menustate *state, char* label) {
	menuentry *entry = init_entry(label, state->last_entry);
	if (!state->first_entry) {
		state->first_entry = entry;
	}
	state->last_entry = entry;
}

#ifndef __U_BOOT__
int main(void) {
#else
static int do_demo_simplemenu(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]) {
#endif
	menustate state = {
		.last_entry = NULL,
		.first_entry = NULL,
	};
	int ch = 0;
	int curr = 1;
	int menu_width = MAX_LEN;
	int menu_height = 0;

	initscr();

	add_entry(&state, "First entry!!");
	add_entry(&state, "Second entry");
	add_entry(&state, "Third entry");
	add_entry(&state, "Fourth entry");

	menu_height = menu_num_entries(&state);

	// Creates the "canvas" for the menu, centered.
	state.window = newwin(
		menu_height + 2 + 2,
		menu_width + 4,
		0, 0
	);

	state.menu = derwin(
		state.window,
		menu_height,
		menu_width + 2,
		3, 1
	);
	syncok(state.menu, TRUE);

	mvwin(
		state.window,
		(LINES-menu_height-2)/2,
		(COLS-menu_width)/2
	);

	// Draw a box around the window
	box(state.window, 0, 0);
	mvwaddch(state.window, 2, 0, ACS_LTEE);
	whline(state.window, 0, menu_width+2);
	mvwaddch(state.window, 2, menu_width+3, ACS_RTEE);

	// Silence inputs
	noecho();
	curs_set(0);

	// Enables key-based navigation
	keypad(state.window, TRUE);

	printmenu(&state);

	while((ch = wgetch(state.window)) != 'q'){

		switch(ch) {
			case KEY_UP:
				redrawitem(&state, curr, FALSE);
				curr--;
				curr = (curr<1) ? menu_num_entries(&state) : curr;
				redrawitem(&state, curr, TRUE);
				break;
			case KEY_DOWN:
				redrawitem(&state, curr, FALSE);
				curr++;
				curr = (curr > menu_num_entries(&state)) ? 1 : curr;
				redrawitem(&state, curr, TRUE);
				break;
			case '\n':
				mvwprintw(state.window, 1, 2, "hi '%d' ", curr);
				break;
		}
	}

	// Cleanup
	delwin(state.menu);
	delwin(state.window);
	wrefresh(stdscr);
	endwin();

	return 0;
}

U_BOOT_CMD(demo_simplemenu, 1, 0, do_demo_simplemenu, "PDCurses simplemenu demo...", "demo_simplemenu")
