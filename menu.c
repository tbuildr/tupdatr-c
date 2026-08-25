#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include "actions.h"
#include "menu.h"

#define MIN_LABEL_COL_WIDTH 8
#define MENU_MARGIN 8 /* "[ ] " prefix + left/right border + breathing room */

static const char *footer_text =
    "up/down move  space toggle  enter run selected  a all  q quit";

/* Column width = longest label, so every description starts exactly one
 * space after the longest label regardless of how long any label is. */
static int compute_label_col_width(void) {
    int max_len = MIN_LABEL_COL_WIDTH;
    for (int i = 0; i < action_count; i++) {
        int label_len = (int)strlen(actions[i].label);
        if (label_len > max_len) max_len = label_len;
    }
    return max_len;
}

static int compute_width(int label_col_width) {
    int max_content = (int)strlen(footer_text);

    for (int i = 0; i < action_count; i++) {
        int content = label_col_width + 1 + (int)strlen(actions[i].description);
        if (content > max_content) max_content = content;
    }

    int width = max_content + MENU_MARGIN;

    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);
    (void)term_h;
    if (term_w > 4 && width > term_w - 4) {
        width = term_w - 4;
    }
    return width;
}

static void draw_menu(WINDOW *win, int highlight, const int *selected, int label_col_width) {
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " tu update menu ");

    for (int i = 0; i < action_count; i++) {
        if (i == highlight) {
            wattron(win, A_REVERSE);
        }
        char mark = selected[i] ? '*' : ' ';
        mvwprintw(win, i + 2, 2, "[%c] %-*s %s",
                  mark, label_col_width, actions[i].label, actions[i].description);
        if (i == highlight) {
            wattroff(win, A_REVERSE);
        }
    }

    mvwprintw(win, action_count + 4, 2, "%s", footer_text);
    wrefresh(win);
}

int run_interactive_menu(int dry_run) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    int height = action_count + 6;
    int label_col_width = compute_label_col_width();
    int width = compute_width(label_col_width);
    WINDOW *win = newwin(height, width, 1, 2);
    keypad(win, TRUE);

    int selected[64] = {0};
    int highlight = 0;
    int ch;
    int quit = 0;

    while (!quit) {
        draw_menu(win, highlight, selected, label_col_width);
        ch = wgetch(win);

        switch (ch) {
            case KEY_UP:
                highlight = (highlight - 1 + action_count) % action_count;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % action_count;
                break;
            case ' ':
                selected[highlight] = !selected[highlight];
                break;
            case 'a':
                for (int i = 0; i < action_count; i++) selected[i] = 1;
                break;
            case '\n':
                quit = 1;
                break;
            case 'q':
                endwin();
                return 0;
        }
    }
    endwin();

    int any_selected = 0;
    for (int i = 0; i < action_count; i++) {
        if (selected[i]) any_selected = 1;
    }
    if (!any_selected) {
        selected[highlight] = 1;
    }

    int overall_status = 0;
    for (int i = 0; i < action_count; i++) {
        if (selected[i]) {
            int status = run_action(i, dry_run);
            if (status != 0) overall_status = status;
        }
    }
    return overall_status;
}
