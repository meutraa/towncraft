WARNINGS := -Wall -Wextra -Wpedantic \
			-Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wconversion -Wstrict-prototypes -Wunused-macros -Wcomments

LDFLAGS := -lm $(shell pkg-config --libs glfw3 gl epoxy)
CFLAGS  := -ggdb3 $(WARNINGS) -O0 -std=c99 $(shell pkg-config --cflags glfw3 gl epoxy)

.PHONY: all

all:
	@$(RM) -r towncraft
	@$(CC) $(CFLAGS) -o towncraft source/* $(LDFLAGS) -Iinclude
