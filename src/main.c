/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "engine.h"
#include "visibility.h"
#include "test_world.h"
#include "world.h"

/* -performs collision detection and response */
/*  sets new xyz  to position of the viewpoint after collision */
/* -implements gravity by updating y position of viewpoint */
/* note that the world coordinates will be the negative value of
   the array indices */
void collisionResponse()
{
  /* your code goes here */

}


/* background process, it is called when there are no other events */
/* -gravity is also implemented here, duplicate of collisionResponse */
/* -for assignment 3, mob control and digging goes here */
void update()
{
  sample_mob_code(); // came with the file; can be replaced

  /* sample use of the dig flag, it is set equal to 1 when the user */
  /*  presses the space bar, you need to reset it to 0 */
  if (dig == 1)
  {
    printf("dig\n");
    dig = 0;
  }

  /* your code goes here */
}


int main(int argc, char* argv[])
{
  /* Initialize the graphics system */
  graphicsInit(&argc, argv);

  if (testWorld == 1)
    build_test_world();
  else
    build_world();

  trimout();
  /* starts the graphics processing loop */
  glutMainLoop();
  
  return 0; 
}

