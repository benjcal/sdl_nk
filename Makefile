FLAGS = $(shell pkg-config --libs --cflags sdl2 cairo)

example: example.c
	gcc $(FLAGS) -o $@ $^
