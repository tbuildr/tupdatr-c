CC = cc
CFLAGS = -Wall -Wextra -std=c11 -D_DEFAULT_SOURCE -MMD -MP
LDLIBS = -lncurses

SRCS = main.c menu.c actions.c
OBJS = $(addprefix build/,$(SRCS:.c=.o))
BIN = build/tu

.DEFAULT_GOAL := $(BIN)

$(BIN): $(OBJS) | build
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LDLIBS)

build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

# Convenience alias so `make tu` always delegates to the real build target,
# even though the binary itself lives at build/tu. PHONY so make never
# mistakes a stray root-level "tu" file for this target being satisfied.
tu: $(BIN)

clean:
	rm -rf build

.PHONY: clean tu

# Auto-generated header dependencies so editing a .h rebuilds dependents.
-include $(OBJS:.o=.d)
