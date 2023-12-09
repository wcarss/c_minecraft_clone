INCLUDE = ../include
SRC_FILES = main.c engine.c perlin.c visibility.c test_world.c world.c network.c
INCLUDE_FILES = $(INCLUDE)/engine.h $(INCLUDE)/perlin.h $(INCLUDE)/visibility.h $(INCLUDE)/test_world.h $(INCLUDE)/world.h $(INCLUDE)/network.h
CFLAGS = -Wall -std=c99 -g -pedantic -Wno-strict-prototypes

# found via xcrun --sdk macosx --show-sdk-path, w/ /System/Library/Frameworks/ appended:
OSX = -F/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks -framework OpenGL -framework GLUT -lm
# in older versions of macosx:
#OSX = -F/System/Library/Frameworks -framework OpenGL -framework GLUT -lm
UBUNTU = -lm -lGLU -lglut -lrt -DUBUNTU

default: ubuntu

osx: stk.o $(INCLUDE_FILES) $(SRC_FILES)
	gcc $(SRC_FILES) stk.o -o ../a3 -I$(INCLUDE) $(OSX) $(CFLAGS)

ubuntu: stk.o $(INCLUDE_FILES) $(SRC_FILES)
	gcc $(SRC_FILES) stk.o -o ../a3 -I$(INCLUDE) $(UBUNTU) $(CFLAGS) 

stk.o: 
	cd stk; make library

clean:
	rm -rf *.o
