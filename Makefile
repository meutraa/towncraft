SDLLIBS=`sdl2-config --cflags --static-libs` -lSDL2_image -lSDL2_ttf -lSDL2_mixer -Wl,-Dynamic -L/usr/lib/x86_64-linux-gnu/pulseaudio -lpulse-simple -lpulse -lpulsecommon-4.0

all: main docs

static:
	$(CC) -g -static -Wall -Wextra -Wno-switch --pedantic -O3 source/*.c -std=c99 -o towncraft $(SDLLIBS)

main:
	$(CC) -g -Wall -Wextra -Wno-switch --pedantic -O3 source/*.c -std=c99 -o towncraft `pkg-config --cflags --libs sdl2` -lSDL2_image -lSDL2_ttf -lSDL2_mixer

docs:
	doxygen documentation/doxygen.cfg
	
valgrind: clean main
	valgrind --leak-check=full ./towncraft

clean:
	rm -rf towncraft
	rm -rf documentation/html
	rm -rf documentation/latex