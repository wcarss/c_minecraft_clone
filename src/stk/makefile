#
# Makefile for Stack ADT
# Written by Wyatt Carss, January 2011
#
# 'make' to cleanly make the sample program cleanly
# 'make library' just to get stack.o for linking
# 'make ugly' to get both
# 'make clean' to get rid of the library 
# 'make cleanall' to get rid of all compiled code
#

FLAGS = -g -Wall -pedantic --std=c99

default: library

fresh: main clean

library: stack.o clean

ugly: main

main: main.o stack.o
	gcc main.o stack.o -o main $(FLAGS)

main.o: main.c
	gcc -c main.c -o main.o $(FLAGS)

stack.o: stack.c
	gcc -c stack.c -o ../stk.o $(FLAGS)

clean:
	rm -f *.o

cleanall: clean
	rm -f main

