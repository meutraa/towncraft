all: main

main:
	gcc -Wall -O3 source/*.c -o towncraft `pkg-config --cflags --libs sdl2`

clean:
	rm -rf source/*.o
	rm -rf towncraft