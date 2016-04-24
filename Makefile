WARNINGS := -Wall -Wextra -Wpedantic \
			-Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wuninitialized -Wconversion -Wstrict-prototypes -Wunused-macros -Wcomments

LDFLAGS := -lm -lSDL2_image -lSDL2_ttf -lSDL2_mixer $(shell pkg-config --libs sdl2)
CFLAGS  := -g $(WARNINGS) -O3 -std=c99 $(shell pkg-config --cflags sdl2)

SRCDIR := source
HDRDIR := include
OBJDIR := objects

SRCFILES := $(shell find $(SRCDIR) -type f -name "*.c" | sed 's/.*\///')
OBJFILES := $(patsubst %.c,%.o,$(SRCFILES))
OBJPATHS := $(patsubst %,$(OBJDIR)/%,$(OBJFILES))
DEPPATHS := $(patsubst %.o,%.d,$(OBJPATHS))

.PHONY: all clean

all: main docs

main: $(OBJPATHS)
	$(CC) -o towncraft $(OBJPATHS) $(LDFLAGS)

# pull in dependency info for *existing* .o files
-include $(DEPPATHS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c Makefile
	@$(CC) -c  $(CFLAGS) $(SRCDIR)/$*.c -o $(OBJDIR)/$*.o -I$(HDRDIR)
	@$(CC) -MM $(CFLAGS) $(SRCDIR)/$*.c >  $(OBJDIR)/$*.d -I$(HDRDIR)
	@cp -f $(OBJDIR)/$*.d $(OBJDIR)/$*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $(OBJDIR)/$*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $(OBJDIR)/$*.d
	@rm -f $(OBJDIR)/$*.d.tmp
	@sed -i "1s/^/$(OBJDIR)\//" $(OBJDIR)/$*.d

clean:
	-$(RM) -r $(OBJDIR)/* towncraft documentation/html documentation/latex

docs:
	doxygen documentation/doxygen.cfg
