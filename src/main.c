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
  float x, y, z;
  getViewPosition(&x, &y, &z);
  x *= -1;
  y *= -1;
  z *= -1;

  if (flycontrol == 1) { return; }

  if (((world[(int)x][(int)y][(int)z] != 0) && (world[(int)x][(int)y][(int)z] != WHITE)) ||
      ((world[(int)x][(int)y + 1][(int)z] != 0) && (world[(int)x][(int)y + 1][(int)z] != WHITE))) {
    if (world[(int)x][(int)y + 1][(int)z] == 0) {
      y++;
      x *= -1;
      y *= -1;
      z *= -1;
    } else {
      getOldViewPosition(&x, &y, &z);
    }

    setViewPosition(x, y, z);
  }
}

/* background process, it is called when there are no other events */
/* -gravity is also implemented here, duplicate of collisionResponse */
/* -for assignment 3, mob control and digging goes here */
void update()
{
  float vx, vy, vz, mx, my, mz, rx, ry, rz;
  int flattened_rx, flattened_ry, flattened_rz;
  // sample_mob_code(); // came with the file; can be replaced
  static int save = 0;
  static double deg;
  static int x = 0;
  static int y = 0;
  float fx = 0, fy = 0;
  int z = 50;
  //char buf[MESSAGE_LENGTH];

  if (netClient && netServer) {
    fprintf(stderr, "error: client and server at once\n");
  }

  if (netClient == 1) {
    get_stuff_from_server();
    send_stuff_to_server();
  }

  if (netServer == 1) {
    send_stuff_to_clients();
    get_stuff_from_client();
  }

  /* restore a previously saved position */
  world[x][y][z] = save;

  /* calculate position of the light */
  fx = (WORLDX / 2.0) - 1 + (WORLDX / 2.0) * cos(deg * PI / 180.0);
  fy = (WORLDY / 2.0) - 1 + (WORLDY / 2.0) * sin(deg * PI / 180.0);
  x = round(fx);
  y = round(fy);

  /* save the state of the cube the light will fill */
  save = world[x][y][z];

  /* place the light and the white cube */
  world[x][y][z] = WHITE;
  setLightPosition(x, y - 1, z);
  sun_flag = 1;
  deg += 0.1;

  /* using this as a timestep for the light and clouds */
  if (deg > 180) { deg = 0; }

  /* Every few timesteps, update the clouds */
  if ((int)(deg * 100) % 3 == 0) {
    perlin_clouds(90, 8, deg);
    clouds_flag = 1;
  }

  mob_action();

  /* sample use of the dig flag, it is set equal to 1 when the user */
  /*  presses the space bar, you need to reset it to 0 */
  getViewOrientation(&mx, &my, &mz);
  getViewPosition(&vx, &vy, &vz);

  if (dig == 1) {
    printf("vx,vy,vz: %.2f,%.2f,%.2f\n", vx, vy, vz);
    printf("(id) px,py,pz: (%d) %.2f,%.2f,%.2f\n", identity, playerPosition[identity][0], playerPosition[identity][1], playerPosition[identity][2]);
  }

  vx *= -1;
  vy *= -1;
  vz *= -1;

  while (mx >= 360) { mx -= 360; }

  // my can be outside of 360 negatively or positively
  while (my >= 360) { my -= 360; }

  while (my <= -360) { my += 360; }

  while (mz >= 360) { mz -= 360; }

  float rad_y = my / 180 * PI;
  float rad_x = mx / 180 * PI;
  const float r = 0.2; // distance to walk along the line
  // basis-vector components:
  float x_component = sin(rad_y) * cos(rad_x); // sin-y, tempered by up+down angle
  float y_component = -1 * sin(rad_x); // negatively impacted by up+down, 0 at 0 and 1.0 at -90
  float z_component = - cos(rad_y) * cos(rad_x); // -cos-y, tempered by up+down angle
  //printf("x,y,z components: %f, %f, %f:\n", x_component, y_component, z_component);
  // initialize the walk state
  rx = vx + r * x_component;
  ry = vy + 0.5 + r * y_component;
  rz = vz + r * z_component;

  int i;

  // walk along the line, checking for intersections:
  for (i = 0; i < 50; i++) {
    rx = rx + r * x_component;
    ry = ry + r * y_component;
    rz = rz + r * z_component;
    flattened_rx = (int)floor(rx);
    flattened_ry = (int)floor(ry);
    flattened_rz = (int)floor(rz);

    if (world[flattened_rx][flattened_ry][flattened_rz]) {
      highlight[0] = flattened_rx;
      highlight[1] = flattened_ry;
      highlight[2] = flattened_rz;
      break;
    }
  }

  if (dig == 1) {
    digflag[0] = 1;
    world[highlight[0]][highlight[1]][highlight[2]] = EMPTY;
    trimout();
    digflag[1] = flattened_rx;
    digflag[2] = flattened_ry;
    digflag[3] = flattened_rz;
    printf("dug: rx, ry, rz: %d, %d, %d\n", flattened_rx, flattened_ry, flattened_rz);

    dig = 0;
  }

  if (flycontrol == 0) {
    getViewPosition(&vx, &vy, &vz);
    vx *= -1;
    vy *= -1;
    vz *= -1;

    if (world[(int)vx][(int)(vy - 1.6)][(int)vz] == 0) {
      vy -= 0.35;
      vx *= -1;
      vy *= -1;
      vz *= -1;
      setViewPosition(vx, vy, vz);
      setPlayerPosition(identity, vx, vy, vz, mx, my);
    }
  }

  buildDisplayList();
  glutPostRedisplay();
}

void parseArgs(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-full") == 0) {
      fullscreen = 1;
    }

    if (strcmp(argv[i], "-drawall") == 0) {
      displayAllCubes = 1;
    }

    if (strcmp(argv[i], "-testworld") == 0) {
      testWorld = 1;
    }

    if (strcmp(argv[i], "-showFPS") == 0 || strcmp(argv[i], "-showfps") == 0 || strcmp(argv[i], "-show-fps") == 0) {
      showFPS = 1;
    }

    if (strcmp(argv[i], "-client") == 0) {
      netClient = 1;
    }

    if (strcmp(argv[i], "-server") == 0) {
      netServer = 1;
    }

    // read random seed if supplied
    if (strcmp(argv[i], "-seed") == 0) {
      if (argc <= i+1) {
        printf("-seed passed without required INT parameter");
        exit(1);
      }
      sscanf(argv[i+1], "%d", &randomSeed);
      randomSeedGiven = 1;
      printf("set seed to %d\n", randomSeed);
    }

    if (strcmp(argv[i], "-help") == 0) {
      printf("Usage: a4 [-full] [-drawall] [-testworld] [-showFPS] [-client] [-server] [-seed INT]\n");
      exit(0);
    }
  }
}


int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  parseArgs(argc, argv);

  initPlayerArray();
  initMobArray();

  if (!randomSeedGiven) {
    randomSeed = time(NULL);
  }
  printf("setting random randomSeed: %d\n", randomSeed);
  srand(randomSeed);

  if (netClient) {
    server_socket = client_setup();

    while (1) {
      printf("trying...\n");

      if (load_game_over_network(server_socket) == 0) { break; }
    }
  } else if (netServer) {
    server_socket = server_setup();
  }

  createPlayer(identity, -50, -80, -50, 0, 0);
  // if you're visible to yourself you'll block your camera
  hidePlayer(identity);

  if (!netClient) {
    if (testWorld == 1) {
      printf("building test world\n");
      build_test_world();
    } else {
      printf("building main world\n");
      build_world();
      place_mobs();
    }
  }


  trimout();
  char title[64];
  sprintf(title, "%s %s %d", argv[0], netClient ? "client" : netServer ? "server" : "standalone", identity);
  initializeOpenGL(title);

  return 0;
}

