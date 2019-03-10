#include <math.h>
#include <stdio.h>

#include "engine.h"
#include "visibility.h"

int you_can_see(int x, int y, int z)
{
  if(x == 0 || y == 0 || z == 0)
    return 0;
  if(x == WORLDX-1 || y == WORLDY-1 || z == WORLDZ-1)
    return 0;

  if(world[x+1][y][z] == 0 || world[x-1][y][z] == 0
      || world[x][y+1][z] == 0 || world[x][y-1][z] == 0
      || world[x][y][z+1] == 0 || world[x][y][z-1] == 0)
    return 1;
  else
    return 0;
}

void trimout()
{
  int debug = 0;
  int x,y,z;
  
  for(x = 0; x < WORLDX; x++)
  {
    for(y = 0; y < WORLDY; y++)
    {
      for(z = 0; z < WORLDZ; z++)
      {
        if(you_can_see(x,y,z))
        {
          visible[x][y][z] = 1;
        }
        else
        {
          visible[x][y][z] = 0;
        }

        if(y == 18 && (x == WORLDX-1 || z == WORLDZ-1 || x == 0 || z == 0))
          visible[x][y][z] = 1;

        if(debug)
          if(x == 0 || y == 0 || z == 0 || x == WORLDX-1 || y == WORLDY-1 || z == WORLDZ-1)
            visible[x][y][z] = 0;
      }
    }
  }	
}




/* determines which cubes are to be drawn and puts them into */
/* the displayList  */
/* write your cube culling code here */
void buildDisplayList()
{
  int i, j, k, l;
  displayCount = 0;

  ExtractFrustum();

  for(i = 0; i < totalDisplayCount; i++) {
    j = totalDisplayList[i][0];
    k = totalDisplayList[i][1];
    l = totalDisplayList[i][2];

    if (CubeInFrustum(j, k, l, 0.5) && world[j][k][l] != 0) {
      addDisplayList(j, k, l);
    }
  }

  /* redraw the screen at the end of the update */
  glutPostRedisplay();
}

void buildTotalDisplayList()
{
  int i, j, k;

  totalDisplayCount = 0;
  for(i = 0; i < WORLDX; i++)
    for(j = 0; j < WORLDY; j++)
      for(k = 0; k < WORLDZ; k++)
        if (visible[i][j][k] == 1 && world[i][j][k] != 0)
          addTotalDisplayList(i, j, k);

  printf("total display count: %i\n", totalDisplayCount);
}

int CubeInFrustum( float x, float y, float z, float size )
{
  int p;
  int v = -1;

  float xm = 0;
  float ym = 0;
  float zm = 0;

  float xp = 0;
  float yp = 0;
  float zp = 0;

  for( p = 0; p < 6; p++ )
  {
    xm = frustum[p][0] * (x - size);
    ym = frustum[p][1] * (y - size);
    zm = frustum[p][2] * (z - size);

    xp = frustum[p][0] * (x + size);
    yp = frustum[p][1] * (y + size);
    zp = frustum[p][1] * (z + size);

    if( xm + ym + zm + frustum[p][3] > v){
      continue;
    }
    if( xp + ym + zm + frustum[p][3] > v) {
      continue;
    }
    if( xm + yp + zm + frustum[p][3] > v) {
      continue;
    }
    if( xp + yp + zm + frustum[p][3] > v) {
      continue;
    }
    if( xm + ym + zp + frustum[p][3] > v) {
      continue;
    }
    if( xp + ym + zp + frustum[p][3] > v) {
      continue;
    }
    if( xm + yp + zp + frustum[p][3] > v) {
      continue;
    }
    if( xp + yp + zp + frustum[p][3] > v) {
      continue;
    }

    return 0;
  }

  return 1;
}


void ExtractFrustum()
{
   float   proj[16];
   float   modl[16];
   float   clip[16];
   float   t;

   /* Get the current PROJECTION matrix from OpenGL */
   glGetFloatv( GL_PROJECTION_MATRIX, proj );

   /* Get the current MODELVIEW matrix from OpenGL */
   glGetFloatv( GL_MODELVIEW_MATRIX, modl );

   /* Combine the two matrices (multiply projection by modelview) */
   clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
   clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
   clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
   clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

   clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
   clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
   clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
   clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

   clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
   clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
   clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
   clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

   clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
   clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
   clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
   clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

   /* Extract the numbers for the RIGHT plane */
   frustum[0][0] = clip[ 3] - clip[ 0];
   frustum[0][1] = clip[ 7] - clip[ 4];
   frustum[0][2] = clip[11] - clip[ 8];
   frustum[0][3] = clip[15] - clip[12];

   /* Normalize the result */
   t = sqrt( frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2] );
   frustum[0][0] /= t;
   frustum[0][1] /= t;
   frustum[0][2] /= t;
   frustum[0][3] /= t;

   /* Extract the numbers for the LEFT plane */
   frustum[1][0] = clip[ 3] + clip[ 0];
   frustum[1][1] = clip[ 7] + clip[ 4];
   frustum[1][2] = clip[11] + clip[ 8];
   frustum[1][3] = clip[15] + clip[12];

   /* Normalize the result */
   t = sqrt( frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2] );
   frustum[1][0] /= t;
   frustum[1][1] /= t;
   frustum[1][2] /= t;
   frustum[1][3] /= t;

   /* Extract the BOTTOM plane */
   frustum[2][0] = clip[ 3] + clip[ 1];
   frustum[2][1] = clip[ 7] + clip[ 5];
   frustum[2][2] = clip[11] + clip[ 9];
   frustum[2][3] = clip[15] + clip[13];

   /* Normalize the result */
   t = sqrt( frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2] );
   frustum[2][0] /= t;
   frustum[2][1] /= t;
   frustum[2][2] /= t;
   frustum[2][3] /= t;

   /* Extract the TOP plane */
   frustum[3][0] = clip[ 3] - clip[ 1];
   frustum[3][1] = clip[ 7] - clip[ 5];
   frustum[3][2] = clip[11] - clip[ 9];
   frustum[3][3] = clip[15] - clip[13];

   /* Normalize the result */
   t = sqrt( frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2] );
   frustum[3][0] /= t;
   frustum[3][1] /= t;
   frustum[3][2] /= t;
   frustum[3][3] /= t;

   /* Extract the FAR plane */
   frustum[4][0] = clip[ 3] - clip[ 2];
   frustum[4][1] = clip[ 7] - clip[ 6];
   frustum[4][2] = clip[11] - clip[10];
   frustum[4][3] = clip[15] - clip[14];

   /* Normalize the result */
   t = sqrt( frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2] );
   frustum[4][0] /= t;
   frustum[4][1] /= t;
   frustum[4][2] /= t;
   frustum[4][3] /= t;

   /* Extract the NEAR plane */
   frustum[5][0] = clip[ 3] + clip[ 2];
   frustum[5][1] = clip[ 7] + clip[ 6];
   frustum[5][2] = clip[11] + clip[10];
   frustum[5][3] = clip[15] + clip[14];

   /* Normalize the result */
   t = sqrt( frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2] );
   frustum[5][0] /= t;
   frustum[5][1] /= t;
   frustum[5][2] /= t;
   frustum[5][3] /= t;
}

int PointInFrustum( float x, float y, float z )
{
   int p;

   for( p = 0; p < 6; p++ )
      if( frustum[p][0] * x + frustum[p][1] * y + frustum[p][2] * z + frustum[p][3] <= 0 )
         return 0;
   return 1;
}
