#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "actions.h"
#include "menu.h"

static void print_usage(const char *prog) {
    printf("Usage: %s [options]\n\n", prog);
    printf("With no options, opens the interactive menu.\n\n");
    printf("Options:\n");
    printf("  --all              run every update action\n");
    for (int i = 0; i < action_count; i++) {
        printf("  --%-16s %s\n", actions[i].key, actions[i].description);
    }
    printf("  --dry-run          print commands instead of running them\n");
    printf("  --list             list available actions and exit\n");
    printf("  -h, --help         show this help\n");
}

static void list_actions(void) {
    for (int i = 0; i < action_count; i++) {
        printf("%-12s %s\n", actions[i].key, actions[i].description);
    }
}

int main(int argc, char **argv) {
    int dry_run = 0;
    int do_all = 0;
    int do_list = 0;
    int requested[64] = {0};
    int any_requested = 0;

    struct option long_opts[64];
    int n = 0;
    for (; n < action_count; n++) {
        long_opts[n].name = actions[n].key;
        long_opts[n].has_arg = no_argument;
        long_opts[n].flag = NULL;
        long_opts[n].val = 1000 + n;
    }
    long_opts[n++] = (struct option){"all", no_argument, NULL, 'A'};
    long_opts[n++] = (struct option){"dry-run", no_argument, NULL, 'D'};
    long_opts[n++] = (struct option){"list", no_argument, NULL, 'L'};
    long_opts[n++] = (struct option){"help", no_argument, NULL, 'h'};
    long_opts[n++] = (struct option){NULL, 0, NULL, 0};

    int opt;
    while ((opt = getopt_long(argc, argv, "h", long_opts, NULL)) != -1) {
        if (opt >= 1000 && opt < 1000 + action_count) {
            requested[opt - 1000] = 1;
            any_requested = 1;
        } else switch (opt) {
            case 'A': do_all = 1; break;
            case 'D': dry_run = 1; break;
            case 'L': do_list = 1; break;
            case 'h': print_usage(argv[0]); return 0;
            default:  print_usage(argv[0]); return 1;
        }
    }

    if (do_list) {
        list_actions();
        return 0;
    }

    if (do_all) {
        int overall = 0;
        for (int i = 0; i < action_count; i++) {
            int status = run_action(i, dry_run);
            if (status != 0) overall = status;
        }
        return overall;
    }

    if (any_requested) {
        int overall = 0;
        for (int i = 0; i < action_count; i++) {
            if (requested[i]) {
                int status = run_action(i, dry_run);
                if (status != 0) overall = status;
            }
        }
        return overall;
    }

    return run_interactive_menu(dry_run);
}
