#include "engine.h"
#include "test_world.h"
#include <stdlib.h>

void sample_mob_code()
{  /* sample code showing the position and rotation controls */
   /* for the mobs, this can be removed */
   if (testWorld)
   {
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
      setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

      /* move mob 0 in the x axis */
      if (increasingmob0 == 1)
	 mob0x += 0.2;
      else 
	 mob0x -= 0.2;
      if (mob0x > 50) increasingmob0 = 0;
      if (mob0x < 30) increasingmob0 = 1;

      /* rotate mob 0 around the y axis */
      mob0ry += 1.0;
      if (mob0ry > 360.0) mob0ry -= 360.0;

      /* move mob 1 and rotate */
      setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

      /* move mob 1 in the z axis */
      /* when mob is moving away it is visible, when moving back it */
      /* is hidden */
      if (increasingmob1 == 1)
      {
	 mob1z += 0.2;
	 showMob(1);
      }
      else
      {
	 mob1z -= 0.2;
	 hideMob(1);
      }
      if (mob1z > 72) increasingmob1 = 0;
      if (mob1z < 52) increasingmob1 = 1;

      /* rotate mob 1 around the y axis */
      mob1ry += 1.0;
      if (mob1ry > 360.0) mob1ry -= 360.0;
   }
}

void place_mobs()
{ 
   createMob(0, 45.0, 18.0, 45.0, 0.0);
   createMob(1, 45.0, 18.0, 45.0, 90.0);
   createMob(2, 45.0, 18.0, 45.0, 180.0);
   createMob(3, 45.0, 18.0, 45.0, 360.0);  
}

void mob_action()
{
  static float mobx[4] = {45,45,45,45};
  static float moby[4] = {18,18,18,18};
  static float mobz[4] = {45,45,45,45};
  static float mobr[4] = {0,90,180,270};
  int outcome;
  //float old_mobx, old_moby;

  int i;
  for(i = 0; i < 4; i++)
  {
//    do
//    {
  //    old_mobx = mobx[i];
  //    old_moby = moby[i];
      outcome = rand()%10;
      if(outcome > 1)
        mobx[i] += 1;
        if(mobx[i] >= WORLDX-1) mobx[i] -= 1;
      else if(outcome > 4)
        mobx[i] -= 1;
        if(mobx[i] < 0) mobx[i] += 1;
      else if(outcome > 7)
        moby[i] += 1;
        if(moby[i] >= WORLDY-1) moby[i] -= 1;
      else
        moby[i] -= 1;
        if(moby[i] < 0) moby[i] += 1;
//    }while((old_mobx == mobx[i] && old_moby == moby[i]) || world[(int)mobx[i]][(int)moby[i]][(int)mobz[i]] != EMPTY);
 
    setMobPosition(i, mobx[i], moby[i], mobz[i], mobr[i]);
  }
}


void build_test_world()
{
   int i, j, k;

   /* initialize world to empty */
   for(i=0; i<100; i++)
      for(j=0; j<50; j++)
	 for(k=0; k<100; k++)
	    world[i][j][k] = 0;

   /* some sample objects */
   /* create some green and blue cubes */
   world[50][25][50] = 1;
   world[49][25][50] = 1;
   world[49][26][50] = 1;
   world[52][25][52] = 2;
   world[52][26][52] = 2;


   /* red platform */
   for(i=0; i<100; i++)
   {
      for(j=0; j<100; j++)
      {
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
   for(i=0; i<99; i++)
   {
      world[0][25][i] = 2;
      world[i][25][0] = 2;
      world[99][25][i] = 2;
      world[i][25][99] = 2;
   }

   /* create two sample mobs */
   createMob(0, 50.0, 25.0, 52.0, 0.0);
   createMob(1, 50.0, 25.0, 52.0, 0.0);

   /* create sample player */
   createPlayer(0, 52.0, 27.0, 52.0, 0.0);
} 
