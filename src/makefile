INCLUDE = ../include
SRC_FILES = main.c engine.c perlin.c visibility.c test_world.c world.c
INCLUDE_FILES = $(INCLUDE)/engine.h $(INCLUDE)/perlin.h $(INCLUDE)/visibility.h $(INCLUDE)/test_world.h $(INCLUDE)/world.h
CFLAGS = -Wall -std=c99 -pedantic -g

OSX = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm
UBUNTU = -lm -lGLU -lglut -lrt -DUBUNTU

default: ubuntu

osx: $(INCLUDE_FILES) $(SRC_FILES)
	gcc $(SRC_FILES) -o ../a3 -I$(INCLUDE) $(OSX) $(CFLAGS)

ubuntu: $(INCLUDE_FILES) $(SRC_FILES)
	gcc $(SRC_FILES) -o ../a3 -I$(INCLUDE) $(UBUNTU) $(CFLAGS) 
