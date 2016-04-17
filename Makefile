all: main docs

main:
	gcc -Wall -O3 source/*.c -std=c99 -o towncraft `pkg-config --cflags --libs sdl2` -lSDL2_image

docs:
	doxygen documentation/doxygen.cfg

clean:
	rm -rf towncraft
	rm -rf documentation/html
	rm -rf documentation/latex