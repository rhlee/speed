all:
	gcc -g speed.c -lm -lpthread -o speed -Wall -Wno-pointer-sign
