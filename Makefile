main: main.c
	gcc -g main.c `sdl2-config --cflags --libs` -o main -lm -lSDL2_ttf 
