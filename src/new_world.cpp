#include <stdlib.h>
#include <new_core.h>
#include <new_world.h>
#include <perlin.h>

/* Takes a number and truncs it into the bounds provided. */
double bounds(double number, double low, double high)
{
  if (number > high) { number = high; }

  if (number < low) { number = low; }

  return number;
}

void fill(GameContext *gc, int low, int high, int type)
{
  for (int i = 0; i < WORLDX; i++) {
    for (int j = low; j <= high; j++) {
      for (int k = 0; k < WORLDZ; k++) {
        gc->world[i][j][k] = type;
      }
    }
  }
}

void perlin(GameContext *gc, int low, int high, int type)
{
  int level = (high + low) / 2;
  int scale = level - low;

  for (int x = 0; x < WORLDX; x++) {
    for (int z = 0; z < WORLDZ; z++) {
      /* Give it small x/z values to increase smoothness; 2 octaves is enough */
      int y = level + (scale * PerlinNoise2D(x / 47.0, z / 47.0, 2, 2, 2));

      /* Have to keep the result safely inside the boundaries of the earth. */
      y = bounds(y, 0, WORLDY - 1);

      for (int i = y; gc->world[x][i][z] == EMPTY; i--) {
        gc->world[x][i][z] = type;
      }
    }
  }
}

void cover(GameContext *gc, int type)
{
  for (int i = 0; i < WORLDX; i++) {
    for (int k = 0; k < WORLDZ; k++) {
      for (int j = WORLDY - 1; j > 0; j--) {
        if (gc->world[i][j][k] != EMPTY) {
          gc->world[i][j + 1][k] = type;
          break;
        }
      }
    }
  }
}

void fill_lakes(GameContext *gc, int level)
{
  for (int i = 0; i < WORLDX; i++) {
    for (int k = 0; k < WORLDZ; k++) {
      for (int j = WORLDY - 1; j > 0; j--) {
        if (j <= level && gc->world[i][j][k] == EMPTY) {
          gc->world[i][j][k] = BLUE;
        } else if (gc->world[i][j][k] != EMPTY) {
          break;
        }
      }
    }
  }
}

void cut(GameContext *gc, int x, int z, int level)
{
  for (int i = level; i < WORLDY; i++) {
    gc->world[x][i][z] = EMPTY;
  }
}

void mark(GameContext *gc, int x, int z, int type)
{
  for (int i = WORLDY - 1; i > 0; i--) {
    if (gc->world[x][i][z] != EMPTY) {
      gc->world[x][i + 1][z] = type;
      break;
    }
  }
}

void maze(GameContext *gc, int low, int high)
{
  int dimension_increment = 0;

  for (int x = 1; x < WORLDX - 1; x++) {
    for (int y = low; y <= high; y++) {
      for (int z = 1; z < WORLDZ - 1; z++) {
        if (0 < PerlinNoise3D(x / 12.0, y / 12.0, z / 12.0, 2.1, 1.9, 2)) {
          if (rand() % 3 > 1) {
            gc->world[x][y][z] = STONE;
          } else {
            gc->world[x][y][z] = DIRT;
          }
        }
      }
    }
  }

  for (dimension_increment = 0; dimension_increment <= 1; dimension_increment++) {
    for (int x = 0; x < WORLDX; x++) {
      for (int y = low; y <= high; y++) {
        if (dimension_increment == 0) {
          gc->world[x][y][0] = STONE;
          gc->world[x][y][WORLDZ - 1] = STONE;
        } else {
          gc->world[0][y][x] = STONE;
          gc->world[WORLDX - 1][y][x] = STONE;
        }
      }
    }

    int lines = rand() % 15;
    int x = 0;

    for (int num = 0; num < lines; num++) {
      if (num == 0) {
        x = 45;
      } else {
        x = rand() % (WORLDX - 1);
      }

      for (int z = 1; z < WORLDZ - 1; z++) {
        for (int y = low; y <= high; y++) {
          if (dimension_increment == 0) {
            gc->world[x][y][z] = EMPTY;
          } else if (dimension_increment == 1) {
            gc->world[z][y][x] = EMPTY;
          }
        }
      }
    }
  }
}

/* Makes perlin-noise clouds! Uses Ken Perlin's PerlinNoise3D function, taken from
http://local.wasp.uwa.edu.au/~pbourke/texture_colour/perlin */
void perlin_clouds(GameContext *gc, double level, double scale, double deg)
{
  float light[3] = {0, 0, 0}; // TODO: fix this -- was `getLightPosition();`
  level = bounds(level, 0, WORLDY - 1);

  for (int x = 0; x < WORLDX; x++) {
    for (int z = 0; z < WORLDZ; z++) {
      /* Overwrite whatever was here before, as long as it isn't the sun. */
      if (!((float)x == light[0] && level == light[1] && (float)z == light[2])) {
        gc->world[x][(int)level][z] = EMPTY;
      }

      /* Using deg (from update) as the y-axis, step through slices of
      smooth volumetric noise to get realistic looking clouds. */
      if ((PerlinNoise3D(x / 76.0, z / 76.0, deg / 108.0, 1.99, 2.1, 4) * scale) > 0) {
        gc->world[x][(int)level][z] = WHITE;
      }
    }
  }
}


int build_world(GameContext *gc)
{
//  fill(0,5,BLACK);
//  fill(8,8,COAL);
//  fill(11,11,COAL);
//  fill(14,14,STONE);
  fill(gc, 17, 17, STONE);
  fill(gc, 21, 21, STONE);
  perlin(gc, 21, 30, STONE);
  perlin(gc, 25, 60, DIRT);
  cover(gc, GREEN);
  fill_lakes(gc, 42);
  mark(gc, 45, 44, RED);
  mark(gc, 44, 45, RED);
  mark(gc, 46, 45, RED);
  mark(gc, 45, 46, RED);
  maze(gc, 18, 20);
  cut(gc, 45, 45, 18);
  return 0;
}


int build_little_world(GameContext *gc)
{
//  fill(0,5,BLACK);
//  fill(8,8,COAL);
//  fill(11,11,COAL);
//  fill(14,14,STONE);
  fill(gc, 0, 1, STONE);
  perlin(gc, 2, 4, DIRT);
  return 0;
}
