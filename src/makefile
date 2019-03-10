INCLUDE = ../include
SRC_FILES = main.c engine.c perlin.c visibility.c test_world.c world.c network.c
INCLUDE_FILES = $(INCLUDE)/engine.h $(INCLUDE)/perlin.h $(INCLUDE)/visibility.h $(INCLUDE)/test_world.h $(INCLUDE)/world.h $(INCLUDE)/network.h
CFLAGS = -Wall -std=c99 -g -pedantic

OSX = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm
#OSX = -F/System/Library/Frameworks -framework OPENGL -L/usr/local/lib/ -lGLUT -lm
UBUNTU = -lm -lGL -lGLU -lglut -lrt -DUBUNTU

default: ubuntu

osx: stk.o $(INCLUDE_FILES) $(SRC_FILES)
	gcc $(SRC_FILES) stk.o -o ../a3 -I$(INCLUDE) $(OSX) $(CFLAGS)

ubuntu: stk.o $(INCLUDE_FILES) $(SRC_FILES)
	gcc $(SRC_FILES) stk.o -o ../a3 -I$(INCLUDE) $(UBUNTU) $(CFLAGS) 

stk.o: 
	cd stk; make library

clean:
	rm -rf *.o
