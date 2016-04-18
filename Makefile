all: main docs

main:
	$(CC) -g -Wall -Wextra --pedantic -O3 source/*.c -std=c99 -o towncraft `pkg-config --cflags --libs sdl2` -lSDL2_image -lSDL2_ttf

docs:
	doxygen documentation/doxygen.cfg

clean:
	rm -rf towncraft
	rm -rf documentation/html
	rm -rf documentation/latex