#ifndef MENU_H
#define MENU_H

/* Draws the interactive ncurses menu, lets the user select one or more
 * actions, then runs them. Returns nonzero if any action failed. */
int run_interactive_menu(int dry_run);

#endif
