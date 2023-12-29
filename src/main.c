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

/* background process, it is called when there are no other events */
/* -gravity is also implemented here, duplicate of collisionResponse */
/* -for assignment 3, mob control and digging goes here */
void update()
{
  float vx, vy, vz, mx, my, mz, rx, ry, rz;
  float oldvx, oldvy, oldvz;
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

  const double PLAYER_SPEED = 0.64;
  const double PLAYER_ACCEL = 0.2;
  float rotx = mx / 180.0 * 3.141592;
  float roty = my / 180.0 * 3.141592;
  float oldxspeed = players[identity].speed.x;
  float oldzspeed = players[identity].speed.z;

  if (keyStates['w']) { // forward motion
    players[identity].speed.x += sin(roty) * PLAYER_ACCEL;
    players[identity].speed.z -= cos(roty) * PLAYER_ACCEL;

    // turn off y motion so you can't usually fly
    if (flycontrol == 1) {
      players[identity].speed.y -= sin(rotx) * PLAYER_ACCEL;
    }
  } else if (keyStates['s']) {  // backward motion
    players[identity].speed.x -= sin(roty) * PLAYER_ACCEL;
    players[identity].speed.z += cos(roty) * PLAYER_ACCEL;

    // turn off y motion so you can't usually fly
    if (flycontrol == 1) {
      players[identity].speed.y += sin(rotx) * PLAYER_ACCEL;
    }
  }

  if (keyStates['a']) { // strafe left motion
    players[identity].speed.x -= cos(roty) * PLAYER_ACCEL;
    players[identity].speed.z -= sin(roty) * PLAYER_ACCEL;
  } else if (keyStates['d']) { // strafe right motion
    players[identity].speed.x += cos(roty) * PLAYER_ACCEL;
    players[identity].speed.z += sin(roty) * PLAYER_ACCEL;
  }

  float pxspeed_squared = players[identity].speed.x*players[identity].speed.x;
  float pzspeed_squared = players[identity].speed.z*players[identity].speed.z;
  float playerspeed_squared = PLAYER_SPEED*PLAYER_SPEED;
  // limit max-speed in any direction to PLAYER_SPEED
  if (pxspeed_squared + pzspeed_squared > playerspeed_squared) {
    players[identity].speed.x = oldxspeed;
    players[identity].speed.z = oldzspeed;
  }

  oldvx = vx;
  oldvy = vy;
  oldvz = vz;

  vx += players[identity].speed.x;
  vy += players[identity].speed.y;
  vz += players[identity].speed.z;

  const float MAX_FALL_SPEED = 1;
  int ix = (int)vx;
  int iy = (int)vy;
  int iz = (int)vz;
  int spot_below = world[ix][(int)(iy - 1)][iz];
  int spot = world[ix][iy][iz];
  int spot_above = world[ix][iy+1][iz];
  if (flycontrol == 0) {
    if (spot_below == EMPTY || spot_below == WHITE) {
      players[identity].speed.y -= 0.18;
      if (players[identity].speed.y > MAX_FALL_SPEED) {
        players[identity].speed.y = MAX_FALL_SPEED;
      }
    } else if (
      (spot != EMPTY && spot != WHITE) ||
      (spot_above != EMPTY && spot_above != WHITE)
    ) {
      players[identity].speed.y = 0;
      if (spot_above == EMPTY) {
        vy += 1;
      } else {
        // can't move into a blocked spot; can't move up to an empty one
        vx = oldvx;
        vy = oldvy;
        vz = oldvz;
      }
    } else {
      players[identity].speed.y = 0;
    }
  }

  if (!(oldvx == vx && oldvy == vy && oldvz == vz)) {
    setPlayerPosition(identity, vx, vy, vz, mx, my);
  }


  float total_speed = sqrt(players[identity].speed.x*players[identity].speed.x+players[identity].speed.y*players[identity].speed.y+players[identity].speed.z*players[identity].speed.z);
  float normalized_pxspeed = total_speed == 0 ? 0 : players[identity].speed.x/total_speed;
  float normalized_pyspeed = total_speed == 0 ? 0 : players[identity].speed.y/total_speed;
  float normalized_pzspeed = total_speed == 0 ? 0 : players[identity].speed.z/total_speed;
  float decel_constant = -0.14;
  if (fabsf(players[identity].speed.x) <= fabsf(decel_constant*normalized_pxspeed)) {
    players[identity].speed.x = 0;
  } else {
    players[identity].speed.x += decel_constant*normalized_pxspeed;
  }

  if (fabsf(players[identity].speed.y) <= fabsf(decel_constant*normalized_pyspeed)) {
    players[identity].speed.y = 0;
  } else {
    players[identity].speed.y += decel_constant*normalized_pyspeed;
  }

  if (fabsf(players[identity].speed.z) <= fabsf(decel_constant*normalized_pzspeed)) {
    players[identity].speed.z = 0;
  } else {
    players[identity].speed.z += decel_constant*normalized_pzspeed;
  }

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
      players[identity].highlight.x = flattened_rx;
      players[identity].highlight.y = flattened_ry;
      players[identity].highlight.z = flattened_rz;
      break;
    }
  }

  if (dig == 1) {
    digflag[0] = 1;
    world[players[identity].highlight.x][players[identity].highlight.y][players[identity].highlight.z] = EMPTY;
    trimout();
    digflag[1] = flattened_rx;
    digflag[2] = flattened_ry;
    digflag[3] = flattened_rz;
    printf("dug: rx, ry, rz: %d, %d, %d\n", flattened_rx, flattened_ry, flattened_rz);

    dig = 0;
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
  //ignore sigpipes for now
  signal(SIGPIPE, SIG_IGN);

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

  createPlayer(identity, 50, 70, 50, 0, 90);
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

