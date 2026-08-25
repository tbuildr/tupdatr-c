#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "actions.h"

/* --- argv vectors for direct execvp() actions (no shell involved) ------ */
static const char *const distrobox_argv[] = { "distrobox-upgrade", "--all", NULL };
static const char *const flatpak_argv[]   = { "flatpak", "update", "-y", NULL };
static const char *const podman_argv[]    = { "podman", "auto-update", NULL };
static const char *const system_argv[]    = { "rpm-ostree", "upgrade", NULL };

/* Edit these to match your actual setup (project paths, container names,
 * etc). Prefer the argv form (use_shell = 0) whenever the command is a
 * single program call with fixed arguments - only fall back to a shell
 * string (use_shell = 1) when you genuinely need pipes, loops, &&, or
 * command substitution. See the comment in actions.h for details. */
const action_t actions[] = {
    {
        "devenv",
        "Devenv packages",
        "Find every devenv.nix under $HOME and run devenv update in it",
        1, NULL,
        "find \"$HOME\" \\( -name .cache -o -name .git -o -name .var "
        "-o -name .mozilla -o -name .steam -o -name .local "
        "-o -name node_modules -o -name result -o -name templates \\) "
        "-prune -o -type f -name devenv.nix -print | while read -r f; do "
        "d=$(dirname \"$f\"); echo \"== $d ==\"; "
        "(cd \"$d\" && devenv update) || fail=1; "
        "done; exit \"${fail:-0}\""
    },
    {
        "distrobox",
        "Distrobox containers",
        "Upgrade all distrobox containers",
        0, distrobox_argv, NULL
    },
    {
        "firmware",
        "Firmware (fwupd)",
        "Refresh metadata and apply available firmware updates",
        1, NULL,
        "fwupdmgr refresh && fwupdmgr update -y"
    },
    {
        "flatpak",
        "Flatpak apps",
        "Update all installed flatpak apps",
        0, flatpak_argv, NULL
    },
    {
        "flakes",
        "Nix flakes (all)",
        "Find every flake.nix under $HOME and update it",
        1, NULL,
        "find \"$HOME\" \\( -name .cache -o -name .git -o -name .var "
        "-o -name .mozilla -o -name .steam -o -name .local "
        "-o -name node_modules -o -name result -o -name templates \\) "
        "-prune -o -type f -name flake.nix -print | while read -r f; do "
        "d=$(dirname \"$f\"); echo \"== $d ==\"; "
        "nix --extra-experimental-features 'nix-command flakes' flake update --flake \"$d\" "
        "|| fail=1; done; exit \"${fail:-0}\""
    },
    {
        "podman",
        "Podman auto-update",
        "Pull + recreate labeled podman containers",
        0, podman_argv, NULL
    },
    {
        "system",
        "System image (rpm-ostree)",
        "Apply pending rpm-ostree upgrade",
        0, system_argv, NULL
    },
};

const int action_count = (int)(sizeof(actions) / sizeof(actions[0]));

/* Builds a human-readable rendering of an argv[] vector for --dry-run
 * output. Not shell-escaped - it's for display only, never re-executed. */
static void print_argv(const char *const *argv) {
    for (int i = 0; argv[i] != NULL; i++) {
        if (i > 0) printf(" ");
        printf("%s", argv[i]);
    }
}

/* Runs argv[0] directly via fork()+execvp() - no shell is involved, so
 * there is no re-parsing step for any argument to be misread through. */
static int run_argv(const char *const *argv) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid == 0) {
        execvp(argv[0], (char *const *)argv);
        /* execvp only returns on failure */
        fprintf(stderr, "failed to exec %s: ", argv[0]);
        perror(NULL);
        _exit(127);
    }

    int wstatus;
    if (waitpid(pid, &wstatus, 0) < 0) {
        perror("waitpid");
        return -1;
    }
    if (WIFEXITED(wstatus)) return WEXITSTATUS(wstatus);
    if (WIFSIGNALED(wstatus)) {
        fprintf(stderr, "%s terminated by signal %d\n", argv[0], WTERMSIG(wstatus));
        return -1;
    }
    return -1;
}

int run_action(int index, int dry_run) {
    if (index < 0 || index >= action_count) {
        fprintf(stderr, "invalid action index: %d\n", index);
        return -1;
    }

    const action_t *a = &actions[index];

    if (dry_run) {
        printf("[dry-run] %s: ", a->label);
        if (a->use_shell) {
            printf("%s\n", a->command);
        } else {
            print_argv(a->argv);
            printf("\n");
        }
        return 0;
    }

    printf("== %s ==\n", a->label);

    if (a->use_shell) {
        int status = system(a->command);
        if (status == -1) {
            fprintf(stderr, "failed to spawn shell for: %s\n", a->command);
            return -1;
        }
        if (WIFEXITED(status)) return WEXITSTATUS(status);
        fprintf(stderr, "shell terminated by signal %d\n", WTERMSIG(status));
        return -1;
    }

    return run_argv(a->argv);
}
