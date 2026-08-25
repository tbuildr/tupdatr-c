# tu — terminal update menu

A small ncurses menu for running your system-update routines in one place:
devenv projects, Nix flakes, distrobox containers, flatpak apps, firmware,
podman auto-updates, and rpm-ostree system upgrades.

## Dependencies

- A C compiler (`cc` or `gcc`) with C11 support
- `make`
- ncurses (library + headers)

Fedora / Silverblue / Kinoite host:

```sh
rpm-ostree install ncurses-devel make gcc
```

Debian / Ubuntu:

```sh
sudo apt install build-essential libncurses-dev
```

## Build

```sh
make            # builds build/tu
make clean      # removes build/
```

The binary is `build/tu`; `make tu` also works as a convenience alias.

## Usage

With no arguments, `tu` opens the interactive menu:

```sh
./build/tu
```

Keys in the menu:

| Key        | Action                                    |
|------------|-------------------------------------------|
| up / down  | Move the highlight                        |
| space      | Toggle selection of highlighted entry     |
| a          | Select all, or clear all if all selected  |
| enter      | Run every selected entry                  |
| q          | Quit without running anything             |

Pressing enter with nothing selected runs only the highlighted entry.
Selected entries run sequentially after you leave the menu; if any fail,
`tu` exits nonzero.

### Command line

Every action is also exposed as a long flag:

```sh
tu --list                 # list available actions
tu --dry-run --all        # print what would run, run nothing
tu --flatpak --system     # run specific actions
tu --all                  # run everything
tu --help
```

Options:

- `--all` — run every action. Cannot be combined with individual flags.
- `--dry-run` — print each command instead of executing it.
- `--list` — list actions and exit.
- `-h`, `--help` — usage text.

Exit status: `0` on success; otherwise the failing command's exit status
(or nonzero if a command could not be launched).

## Adding or changing actions

All actions live in the `actions[]` table in `actions.c`. Each entry has:

- `key` — flag name (`--key`) and menu identity
- `label` — menu display text
- `description` — one-line explanation shown next to the label
- either an `argv` vector (preferred; executed directly via `execvp`,
  no shell involved) or a shell command string for cases that genuinely
  need pipes, loops, or `&&`

Shell strings must stay fixed compile-time literals; never build them
from runtime input. If the table grows past `MAX_ACTIONS` (64, see
`actions.h`), raise the constant.
