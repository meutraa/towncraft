all: main docs

main: clean
	$(CC) -g -Wall -Wextra -Wno-switch -Wno-unused-result --pedantic -O3 source/*.c -std=c99 -o towncraft `pkg-config --cflags --libs sdl2` -lSDL2_image -lSDL2_ttf -lSDL2_mixer

docs:
	doxygen documentation/doxygen.cfg
	
valgrind: clean main
	valgrind --leak-check=full ./towncraft

clean:
	rm -rf towncraft
	rm -rf documentation/html
	rm -rf documentation/latex