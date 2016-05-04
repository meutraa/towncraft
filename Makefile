WARNINGS := -Wall -Wextra -Wpedantic \
			-Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wconversion -Wstrict-prototypes -Wunused-macros -Wcomments

LDFLAGS := -lGL -lGLU -lm -lSDL2_image -lSDL2_ttf -lSDL2_mixer $(shell sdl2-config --libs)
CFLAGS  := -ggdb3 $(WARNINGS) -O0 -std=c99 $(shell sdl2-config --cflags) -I/usr/include/SDL2

SRCDIR := source
HDRDIR := include
OBJDIR := objects

SRCFILES := $(shell find $(SRCDIR) -type f -name "*.c" | sed 's/.*\///')
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
OBJPATHS := $(patsubst %,$(OBJDIR)/%,$(OBJFILES))
DEPPATHS := $(patsubst %.o,%.d,$(OBJPATHS))

.PHONY: all clean dirs cachegrind tidy format

all: main docs

main: clean dirs $(OBJPATHS)
	clang -o towncraft $(OBJPATHS) $(LDFLAGS)

# pull in dependency info for *existing* .o files
-include $(DEPPATHS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c Makefile
	@clang -c $(CFLAGS) $(SRCDIR)/$*.c -o $(OBJDIR)/$*.o -I$(HDRDIR)
	@clang -MM $(CFLAGS) $(SRCDIR)/$*.c >  $(OBJDIR)/$*.d -I$(HDRDIR)
	@cp -f $(OBJDIR)/$*.d $(OBJDIR)/$*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $(OBJDIR)/$*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $(OBJDIR)/$*.d
	@rm -f $(OBJDIR)/$*.d.tmp
	@sed -i "1s/^/$(OBJDIR)\//" $(OBJDIR)/$*.d

clean:
	@$(RM) -r $(OBJDIR)/* towncraft documentation/html documentation/latex callgrind.*

dirs:
	@mkdir -p $(OBJDIR)

cachegrind:
	@clang $(CFLAGS) -o towncraft source/* $(LDFLAGS) -I$(HDRDIR)

tidy:
	clang-tidy source/*.c -checks="*,-google*" -- -Iinclude -I/usr/include/SDL2

docs:
	@doxygen documentation/doxygen.cfg
