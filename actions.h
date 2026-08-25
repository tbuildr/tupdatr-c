#ifndef ACTIONS_H
#define ACTIONS_H

/* Upper bound on entries in actions[]; main() and the menu size their
 * state arrays from this. Raise it (and rebuild) when actions grow. */
#define MAX_ACTIONS 64

typedef struct {
    const char *key;         /* short flag name, e.g. "bootc" */
    const char *label;       /* menu display text */
    const char *description; /* one-line explanation shown in menu */

    /* Exactly one of these two is used, selected by use_shell:
     *
     *   use_shell == 0 (preferred): argv is a NULL-terminated argument
     *   vector executed directly via fork()+execvp(). No shell is ever
     *   invoked, so there is nothing for a shell to reinterpret - safe
     *   even if an argument ever contained shell metacharacters.
     *
     *   use_shell == 1: command is a shell command string, run via
     *   system() (which itself calls `sh -c`). Only used where the
     *   command genuinely needs shell features - pipes, loops, &&,
     *   or command substitution - that a plain argv can't express.
     *   Every command string here is a fixed compile-time literal;
     *   never build one from runtime/user input. */
    int use_shell;
    const char *const *argv;
    const char *command;
} action_t;

extern const action_t actions[];
extern const int action_count;

/* Runs one action by index. Returns the command's exit status.
 * If dry_run is nonzero, prints the command instead of running it. */
int run_action(int index, int dry_run);

#endif
