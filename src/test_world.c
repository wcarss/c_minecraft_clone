#include "engine.h"
#include "test_world.h"
#include <stdlib.h>
#include <math.h>

void sample_mob_code()
{
  /* sample code showing the position and rotation controls */
  /* for the mobs, this can be removed */
  if (testWorld) {
    /* sample of rotation and positioning of mob */
    /* coordinates for mob 0 */
    static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
    static float mob0ry = 0.0;
    static int increasingmob0 = 1;
    /* coordinates for mob 1 */
    static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
    static float mob1ry = 0.0;
    static int increasingmob1 = 1;

    /* move mob 0 and rotate */
    /* set mob 0 position */
    setMobPosition(0, mob0x, mob0y, mob0z, mob0ry, 0);

    /* move mob 0 in the x axis */
    if (increasingmob0 == 1) {
      mob0x += 0.2;
    } else {
      mob0x -= 0.2;
    }

    if (mob0x > 50) { increasingmob0 = 0; }

    if (mob0x < 30) { increasingmob0 = 1; }

    /* rotate mob 0 around the y axis */
    mob0ry += 1.0;

    if (mob0ry > 360.0) { mob0ry -= 360.0; }

    /* move mob 1 and rotate */
    setMobPosition(1, mob1x, mob1y, mob1z, mob1ry, 0);

    /* move mob 1 in the z axis */
    /* when mob is moving away it is visible, when moving back it */
    /* is hidden */
    if (increasingmob1 == 1) {
      mob1z += 0.2;
      showMob(1);
    } else {
      mob1z -= 0.2;
      hideMob(1);
    }

    if (mob1z > 72) { increasingmob1 = 0; }

    if (mob1z < 52) { increasingmob1 = 1; }

    /* rotate mob 1 around the y axis */
    mob1ry += 1.0;

    if (mob1ry > 360.0) { mob1ry -= 360.0; }
  }
}

void place_mobs()
{
  world[46][18][45] = EMPTY;
  world[46][19][45] = EMPTY;
  createMob(0, 46.0, 18.0, 45.0, 0, 0);

  world[44][18][45] = EMPTY;
  world[44][19][45] = EMPTY;
  createMob(1, 44.0, 18.0, 45.0, 0, 0);

  world[45][18][44] = EMPTY;
  world[45][19][44] = EMPTY;
  createMob(2, 45.0, 18.0, 44.0, 0, 0);

  world[45][18][46] = EMPTY;
  world[45][19][46] = EMPTY;
  createMob(3, 45.0, 18.0, 46.0, 0, 0);
}

void mob_action()
{
  float mob_speed = 0.3;
  int outcome;
  int rand_max = 64;
  float old_x, old_y, old_z;
  for (int i = 0; i < 4; i++) {
    outcome = rand() % rand_max; // [0, rand_max) so ends at rand_max - 1

    if (outcome < rand_max - 50) {
      // do nothing 10/64 of the time
    } else if (outcome < rand_max - 4) {
      // move 50/64 of the time
      old_x = mobPosition[i][0];
      old_y = mobPosition[i][1];
      old_z = mobPosition[i][2];
      mobPosition[i][0] += mobSpeed[i][0];
      mobPosition[i][1] += mobSpeed[i][1];
      mobPosition[i][2] += mobSpeed[i][2];
      if (mobPosition[i][0] < 0 || mobPosition[i][0] >= WORLDX - 1 || world[(int)floor(mobPosition[i][0])][(int)floor(mobPosition[i][1])][(int)floor(mobPosition[i][2])] != EMPTY) {
        mobPosition[i][0] = old_x;
      }
      if (mobPosition[i][2] < 0 || mobPosition[i][2] >= WORLDZ - 1 || world[(int)floor(mobPosition[i][0])][(int)floor(mobPosition[i][1])][(int)floor(mobPosition[i][2])] != EMPTY) {
        mobPosition[i][2] = old_z;
      }
      setMobPosition(i, mobPosition[i][0], mobPosition[i][1], mobPosition[i][2], mobPosition[i][3], mobPosition[i][4]);
    } else {
      // recalc direction 4/64 of the time
      if (outcome == rand_max-4) {
        mobSpeed[i][0] = -mob_speed;
        mobSpeed[i][2] = 0;
        mobPosition[i][3] = 270;
      } else if (outcome == rand_max-3) {
        mobSpeed[i][0] = mob_speed;
        mobSpeed[i][2] = 0;
        mobPosition[i][3] = 90;
      } else if (outcome == rand_max-2) {
        mobSpeed[i][0] = 0;
        mobSpeed[i][2] = -mob_speed;
        mobPosition[i][3] = 180;
      } else { // rand_max - 1
        mobSpeed[i][0] = 0;
        mobSpeed[i][2] = mob_speed;
        mobPosition[i][3] = 0;
      }
    }
  }
}

void old_mob_action()
{
  int outcome;
  for (int i = 0; i < 4; i++) {
    outcome = rand() % 12; // 012 345 678 9,10,11

    if (outcome < 3) { // 012
      mobPosition[i][0] += 0.1;
      mobPosition[i][4] = 0;
      if (mobPosition[i][0] >= WORLDX - 1 || world[(int)floor(mobPosition[i][0])][(int)floor(mobPosition[i][1])][(int)floor(mobPosition[i][2])] != EMPTY) {
        mobPosition[i][0] -= 1;
      }
    } else if (outcome < 6) { // 345
      mobPosition[i][0] -= 0.1;
      mobPosition[i][4] = 180;
      if (mobPosition[i][0] < 0 || world[(int)floor(mobPosition[i][0])][(int)floor(mobPosition[i][1])][(int)floor(mobPosition[i][2])] != EMPTY) {
        mobPosition[i][0] += 1;
      }
    } else if (outcome < 9) { // 678
      mobPosition[i][2] += 0.1;
      mobPosition[i][4] = 90;
      if (mobPosition[i][2] >= WORLDZ - 1 || world[(int)floor(mobPosition[i][0])][(int)floor(mobPosition[i][1])][(int)floor(mobPosition[i][2])] != EMPTY) {
        mobPosition[i][2] -= 1;
      }
    } else { // 9 10 11
      mobPosition[i][2] -= 0.1;
      mobPosition[i][4] = 270;
      if (mobPosition[i][2] < 0 || world[(int)floor(mobPosition[i][0])][(int)floor(mobPosition[i][1])][(int)floor(mobPosition[i][2])] != EMPTY) {
        mobPosition[i][2] += 1;
      }
    }

    setMobPosition(i, mobPosition[i][0], mobPosition[i][1], mobPosition[i][2], mobPosition[i][3], mobPosition[i][4]);
  }
}


void build_test_world()
{
  int i, j, k;

  /* initialize world to empty */
  for (i = 0; i < 100; i++)
    for (j = 0; j < 50; j++)
      for (k = 0; k < 100; k++) {
        world[i][j][k] = 0;
      }

  /* some sample objects */
  /* create some green and blue cubes */
  world[50][25][50] = 1;
  world[49][25][50] = 1;
  world[49][26][50] = 1;
  world[52][25][52] = 2;
  world[52][26][52] = 2;


  /* red platform */
  for (i = 0; i < 100; i++) {
    for (j = 0; j < 100; j++) {
      world[i][24][j] = 3;
    }
  }

  /* fill in world under platform - removed for assignment 3*/
  /*
     for(i=0; i<100; i++)
     {
     for(j=0; j<100; j++)
     {
     for(k=0; k<24; k++)
     {
     world[i][k][j] = 3;
     }
     }
     }
   */
  /* blue box shows xy bounds of the world */
  for (i = 0; i < 99; i++) {
    world[0][25][i] = 2;
    world[i][25][0] = 2;
    world[99][25][i] = 2;
    world[i][25][99] = 2;
  }

  /* create two sample mobs */
  createMob(0, 50.0, 25.0, 52.0, 0.0, 0);
  createMob(1, 50.0, 25.0, 52.0, 0.0, 0);

  /* create sample player */
  createPlayer(0, 52.0, 27.0, 52.0, 0.0, 0.0);
}
