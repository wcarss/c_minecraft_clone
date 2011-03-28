/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "engine.h"
#include "visibility.h"
#include "test_world.h"
#include "world.h"
#include "network.h"

/* -performs collision detection and response */
/*  sets new xyz  to position of the viewpoint after collision */
/* -implements gravity by updating y position of viewpoint */
/* note that the world coordinates will be the negative value of
   the array indices */
void collisionResponse()
{
  /* implement collision, gravity, and climbing onto single cubes here */
  float x,y,z;
  getViewPosition(&x,&y,&z);
  x *= -1;
  y *= -1;
  z *= -1;

  if(flycontrol == 1) return;

  if(((world[(int)x][(int)y][(int)z] != 0) && (world[(int)x][(int)y][(int)z] != WHITE)) ||
      ((world[(int)x][(int)y+1][(int)z] != 0) && (world[(int)x][(int)y+1][(int)z] != WHITE)))
  {
    if(world[(int)x][(int)y+1][(int)z] == 0)
    {
      y++;
      x *= -1;
      y *= -1;
      z *= -1;
    }
    else 
    {
      getOldViewPosition(&x,&y,&z);
    }
    setViewPosition(x,y,z);
  }

}


/* background process, it is called when there are no other events */
/* -gravity is also implemented here, duplicate of collisionResponse */
/* -for assignment 3, mob control and digging goes here */
void update()
{
  float vx, vy, vz, mx, my, mz, rx, ry, rz;
//  sample_mob_code(); // came with the file; can be replaced
  static long int last = 0;
  static int save = 0;
  static int deg;
  static int x = 0;
  static int y = 0;
  int z = 50;
  int now;
  //char buf[MESSAGE_LENGTH];

  if(netClient && netServer) fprintf(stderr, "error: client and server at once\n");

  if(netClient == 1)
  {
    get_stuff_from_server();
    send_stuff_to_server();
  }

  if(netServer == 1)
  {
    send_stuff_to_clients();
    get_stuff_from_client();
  }

  /* check your watch */
  now = time(NULL);
  if((now - last) >= 1)
  {		
    /* restore a previously saved position */
    world[x][y][z] = save;

    /* calculate position of the light */
    x = (WORLDX/2.0)-1+(WORLDX/2.0)*cos(deg*PI/180.0);
    y = (WORLDY/2.0)-1+(WORLDY/2.0)*sin(deg*PI/180.0);

    /* save the state of the cube the light will fill */
    save = world[x][y][z];

    /* place the light and the white cube */
    world[x][y][z] = WHITE;
    /*printf("x, y: %d, %d\n", x, y);*/
    setLightPosition(x,y-1,z);
    sun_flag = 1;
    /* using this as a timestep for the light and clouds */
    deg++;
    if(deg > 180) deg = 0;

    /* Every few timesteps, update the clouds */
    if((int)deg % 3 == 0)
    {
      perlin_clouds(90, 8, deg);
      clouds_flag = 1;
      mob_action();
    }
    /* save the time for FPS control */
    last = now;
  }


  /* sample use of the dig flag, it is set equal to 1 when the user */
  /*  presses the space bar, you need to reset it to 0 */
  if (dig == 1)
  {
    digflag[0] = 1;
    getViewOrientation(&mx,&my,&mz);
    getViewPosition(&vx,&vy,&vz);

    vx *= -1;
    vy *= -1;
    vz *= -1;

    while(mx >= 360) mx -= 360;
    while(my >= 360) my -= 360;
    while(mz >= 360) mz -= 360;
    printf("dig: mx, my, mz: %f, %f, %f\t", mx, my, mz);
    printf("pos: vx, vy, vz: %f, %f, %f\t", vx, vy, vz);
    rx = vx+sin(my)*cos(mx);//vx+sin(my)*cos(mx) * 1;
    rz = vz+(-1 * cos(my))*cos(mx);//vz+cos(my)*cos(mx) * 1;
    ry = vy-sin(mx);//(vy+sin(mx) * 1);
    printf("got: rx, ry, rz: %f, %f, %f\t", rx, ry, rz);
    world[(int)round(rx)][(int)round(ry)][(int)round(rz)] = EMPTY;
    trimout();
//    buildDisplayList();
    digflag[1] = -1*(int)round(rx);
    digflag[2] = -1*(int)round(ry);
    digflag[3] = -1*(int)round(rz);
    printf("dug: rx, ry, rz: %d, %d, %d\n", -1*(int)round(rx), -1*(int)round(ry), -1*(int)round(rz));

    dig = 0;
  }

  if(flycontrol == 0)
  {
    getViewPosition(&vx,&vy,&vz);
    vx *= -1;
    vy *= -1;
    vz *= -1;
    if(world[(int)vx][(int)(vy-1.6)][(int)vz] == 0)
    {
      vy -= 0.35;
      vx *= -1;
      vy *= -1;
      vz *= -1;
      setViewPosition(vx,vy,vz);
    }
  }

  /* your code goes here */
}


int main(int argc, char* argv[])
{
  /* Initialize the graphics system */
  graphicsInit(&argc, argv);

  if(!netClient)
  {
    if (testWorld == 1)
      build_test_world();
    else
    {
      build_world();
      place_mobs();
    }
  }

  trimout();
  /* starts the graphics processing loop */
  glutMainLoop();
  
  return 0; 
}

