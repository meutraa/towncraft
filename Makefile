LDFLAGS=-lm -lSDL2_image -lSDL2_ttf -lSDL2_mixer `pkg-config --cflags --libs sdl2`
CFLAGS=-g -Wfatal-errors -Wall -Wextra -Wpedantic -O3 -std=c99 -Wunused-macros -Wcomments

all: main docs

main:
	$(CC) $(CFLAGS) $(LIBS) source/*.c -o towncraft $(LDFLAGS)

docs:
	doxygen documentation/doxygen.cfg

clean:
	rm -rf towncraft
	rm -rf documentation/html
	rm -rf documentation/latex
