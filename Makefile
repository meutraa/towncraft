all: main

main:
	gcc -Wall -O3 source/*.c -o towncraft `pkg-config --cflags --libs sdl2` -lSDL2_image

clean:
	rm -rf towncraft