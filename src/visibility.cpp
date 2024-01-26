#include <math.h>
#include <stdio.h>
#include <new_core.h>
#include <visibility.h>

int you_can_see(GameContext *gc, int x, int y, int z)
{
  if (x == 0 || y == 0 || z == 0) {
    return 0;
  }

  if (x == WORLDX - 1 || y == WORLDY - 1 || z == WORLDZ - 1) {
    return 0;
  }

  if (gc->world[x + 1][y][z] == 0 || gc->world[x - 1][y][z] == 0
      || gc->world[x][y + 1][z] == 0 || gc->world[x][y - 1][z] == 0
      || gc->world[x][y][z + 1] == 0 || gc->world[x][y][z - 1] == 0) {
    return 1;
  } else {
    return 0;
  }
}

void trimout(GameContext *gc)
{
  int debug = 0;
  int x, y, z;

  for (x = 0; x < WORLDX; x++) {
    for (y = 0; y < WORLDY; y++) {
      for (z = 0; z < WORLDZ; z++) {
        if (you_can_see(gc, x, y, z)) {
          gc->visible[x][y][z] = 1;
        } else {
          gc->visible[x][y][z] = 0;
        }

        if (y == 18 && (x == WORLDX - 1 || z == WORLDZ - 1 || x == 0 || z == 0)) {
          gc->visible[x][y][z] = 1;
        }

        if (debug) {
          if (x == 0 || y == 0 || z == 0 || x == WORLDX - 1 || y == WORLDY - 1 || z == WORLDZ - 1) {
            gc->visible[x][y][z] = 0;
          }
        }
      }
    }
  }
}

/* determines which cubes are to be drawn and puts them into */
/* the displayList  */
/* write your cube culling code here */
void buildDisplayList(GameContext *gc, glm::mat4 view, glm::mat4 projection)
{
  /* used to calculate frames per second */
  //static int frame = 0, time, timebase = 0;
  int displayCount = 0;

  trimout(gc);
  ExtractFrustum(gc, view, projection);

  for (int i = 0; i < WORLDX; i++) {
    for (int j = 0; j < WORLDY; j++) {
      for (int k = 0; k < WORLDZ; k++) {
        if (gc->visible[i][j][k] == 1 && gc->world[i][j][k] != 0 && CubeInFrustum(gc, i, j, k, 0.5)) {
          gc->displayList[displayCount][0] = i;
          gc->displayList[displayCount][1] = j;
          gc->displayList[displayCount][2] = k;
          displayCount++;

          if (displayCount > MAX_DISPLAY_LIST) {
            printf("You have put more items in the display list then there are\n");
            printf("cubes in the world. Set displayCount = 0 at some point.\n");
            exit(1);
          }
        }
      }
    }
  }

  gc->displayCount = displayCount;

  /* frame per second calculation */
  /* don't change the following routine */
  /* http://www.lighthouse3d.com/opengl/glut/index.php?fps */
  /*if (showFPS == 1) {
    frame++;
    time = glutGet(GLUT_ELAPSED_TIME);

    if (time - timebase > 1000) {
      printf("FPS:%4.2f\n", frame * 1000.0 / (time - timebase));
      timebase = time;
      frame = 0;
    }
  }*/
}

int CubeInFrustum(GameContext *gc, float x, float y, float z, float size)
{
  int p;
  int c;
  int c2 = 0;
  int v = -2;

  // goodness I hope this works!
  float **frustum = gc->frustum;

  for (p = 0; p < 6; p++) {
    c = 0;

    if (frustum[p][0] * (x - size) + frustum[p][1] * (y - size) + frustum[p][2] * (z - size) + frustum[p][3] > v) {
      c++;
    }

    if (frustum[p][0] * (x + size) + frustum[p][1] * (y - size) + frustum[p][2] * (z - size) + frustum[p][3] > v) {
      c++;
    }

    if (frustum[p][0] * (x - size) + frustum[p][1] * (y + size) + frustum[p][2] * (z - size) + frustum[p][3] > v) {
      c++;
    }

    if (frustum[p][0] * (x + size) + frustum[p][1] * (y + size) + frustum[p][2] * (z - size) + frustum[p][3] > v) {
      c++;
    }

    if (frustum[p][0] * (x - size) + frustum[p][1] * (y - size) + frustum[p][2] * (z + size) + frustum[p][3] > v) {
      c++;
    }

    if (frustum[p][0] * (x + size) + frustum[p][1] * (y - size) + frustum[p][2] * (z + size) + frustum[p][3] > v) {
      c++;
    }

    if (frustum[p][0] * (x - size) + frustum[p][1] * (y + size) + frustum[p][2] * (z + size) + frustum[p][3] > v) {
      c++;
    }

    if (frustum[p][0] * (x + size) + frustum[p][1] * (y + size) + frustum[p][2] * (z + size) + frustum[p][3] > v) {
      c++;
    }

    if (c == 0) {
      return 0;
    }

    if (c == 8) {
      c2++;
    }
  }

  return (c2 == 6) ? 2 : 1;
}


void ExtractFrustum(GameContext *gc, glm::mat4 view, glm::mat4 projection)
{
  float   *proj = (float *)&projection;
  float   *modl = (float *)&view;
  float   clip[16];
  float   t;

  // goodness I hope this works!
  float **frustum = gc->frustum;

  /* Get the current PROJECTION matrix from OpenGL */
  //glGetFloatv(GL_PROJECTION_MATRIX, proj);

  /* Get the current MODELVIEW matrix from OpenGL */
  //glGetFloatv(GL_MODELVIEW_MATRIX, modl);

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
  t = sqrt(frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2]);
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
  t = sqrt(frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2]);
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
  t = sqrt(frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2]);
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
  t = sqrt(frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2]);
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
  t = sqrt(frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2]);
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
  t = sqrt(frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2]);
  frustum[5][0] /= t;
  frustum[5][1] /= t;
  frustum[5][2] /= t;
  frustum[5][3] /= t;
}

int PointInFrustum(GameContext *gc, float x, float y, float z)
{
  for (int p = 0; p < 6; p++) {
    if (gc->frustum[p][0] * x + gc->frustum[p][1] * y + gc->frustum[p][2] * z + gc->frustum[p][3] <= 0) {
      return 0;
    }
  }

  return 1;
}
